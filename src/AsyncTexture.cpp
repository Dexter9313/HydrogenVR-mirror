/*
    Copyright (C) 2019 Florian Cabot <florian.cabot@hotmail.fr>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "AsyncTexture.hpp"

QList<QPair<QFuture<void>, GLHandler::PixelBufferObject>>&
    AsyncTexture::waitingForDeletion()
{
	static QList<QPair<QFuture<void>, GLHandler::PixelBufferObject>>
	    waitingForDeletion = {};
	return waitingForDeletion;
}

AsyncTexture::AsyncTexture(QString const& path, QColor const& defaultColor,
                           bool sRGB)
    : sRGB(sRGB)
{
	unsigned char color[4];
	color[0]   = defaultColor.red();
	color[1]   = defaultColor.green();
	color[2]   = defaultColor.blue();
	color[3]   = defaultColor.alpha();
	defaultTex = GLHandler::newTexture(1, 1, &(color[0]), sRGB);

	if(path.isEmpty())
	{
		emptyPath = true;
		return;
	}

	QImageReader imReader(path);
	QSize size(imReader.size());

	pbo = GLHandler::newPixelBufferObject(size.width(), size.height());
	unsigned char* data(pbo.mappedData);

	future = QtConcurrent::run([path, data]() {
		QImageReader imReader(path);
		QImage img(imReader.read());
		if(img.isNull())
		{
			// NOLINTNEXTLINE(hicpp-no-array-decay)
			qWarning() << "Could not load Texture '" + path
			                  + "' : " + imReader.errorString();
			return;
		}
		img = img.convertToFormat(QImage::Format_RGBA8888);
		std::memcpy(data, img.bits(), std::size_t(img.byteCount()));
	});
}

AsyncTexture::AsyncTexture(QString const& path, unsigned int width,
                           unsigned int height, QColor const& defaultColor,
                           bool sRGB)
    : sRGB(sRGB)
{
	unsigned char color[4];
	color[0]   = defaultColor.red();
	color[1]   = defaultColor.green();
	color[2]   = defaultColor.blue();
	color[3]   = defaultColor.alpha();
	defaultTex = GLHandler::newTexture(1, 1, &(color[0]), sRGB);

	if(path.isEmpty())
	{
		emptyPath = true;
		return;
	}

	pbo = GLHandler::newPixelBufferObject(width, height);
	unsigned char* data(pbo.mappedData);

	future = QtConcurrent::run([path, data, width, height]() {
		QImageReader imReader(path);
		imReader.setScaledSize(QSize(width, height));
		QImage img(imReader.read());
		if(img.isNull())
		{
			// NOLINTNEXTLINE(hicpp-no-array-decay)
			qWarning() << "Could not load Texture '" + path
			                  + "' : " + imReader.errorString();
			return;
		}
		img = img.convertToFormat(QImage::Format_RGBA8888);
		std::memcpy(data, img.bits(), std::size_t(img.byteCount()));
	});
}

GLHandler::Texture AsyncTexture::getTexture()
{
	if(emptyPath)
	{
		return defaultTex;
	}

	if(loaded)
	{
		return tex;
	}

	if(!future.isFinished())
	{
		return defaultTex;
	}

	tex = GLHandler::copyPBOToTex(pbo, sRGB);
	GLHandler::deletePixelBufferObject(pbo);
	loaded = true;

	return tex;
}

AsyncTexture::~AsyncTexture()
{
	GLHandler::deleteTexture(defaultTex);
	if(!emptyPath)
	{
		if(loaded)
		{
			GLHandler::deleteTexture(tex);
		}
		else if(future.isFinished())
		{
			GLHandler::deletePixelBufferObject(pbo);
		}
		else
		{
			waitingForDeletion().push_back({future, pbo});
		}
	}
}

void AsyncTexture::garbageCollect(bool force)
{
	// go in reverse because of possible deletions
	for(int i(waitingForDeletion().size() - 1); i >= 0; --i)
	{
		if(force)
		{
			waitingForDeletion()[i].first.waitForFinished();
		}
		if(waitingForDeletion()[i].first.isFinished())
		{
			GLHandler::deletePixelBufferObject(waitingForDeletion()[i].second);
			waitingForDeletion().removeAt(i);
		}
	}
}
