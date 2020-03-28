/*
    Copyright (C) 2020 Florian Cabot <florian.cabot@hotmail.fr>

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

#include "Renderer.hpp"

#include "AbstractMainWin.hpp"

void Renderer::init(AbstractMainWin* window, VRHandler* vrHandler)
{
	this->window    = /*dynamic_cast<MainWin*>(window);*/ window;
	this->vrHandler = vrHandler;
	if(initialized)
	{
		clean();
	}

	dbgCamera = new DebugCamera(vrHandler);
	dbgCamera->lookAt({2, 0, 2}, {0, 0, 0}, {0, 0, 1});
	dbgCamera->setPerspectiveProj(70.0f, getAspectRatio());

	auto defaultCam = new BasicCamera(vrHandler);
	defaultCam->lookAt({1, 1, 1}, {0, 0, 0}, {0, 0, 1});
	defaultCam->setPerspectiveProj(70.0f, getAspectRatio());
	appendSceneRenderPath("default", RenderPath(defaultCam));

	reloadPostProcessingTargets();

	initialized = true;
}

void Renderer::windowResized()
{
	if(!initialized)
	{
		return;
	}

	QSettings().setValue("window/width", window->size().width());
	QSettings().setValue("window/height", window->size().height());
	if(QSettings().value("window/forcerenderresolution").toBool())
	{
		return;
	}
	for(auto pair : sceneRenderPipeline_)
	{
		pair.second.camera->setPerspectiveProj(70.0f, getAspectRatio());
	}
	dbgCamera->setPerspectiveProj(70.0f, getAspectRatio());
	reloadPostProcessingTargets();
}

QSize Renderer::getSize() const
{
	QSize renderSize(window->size().width(), window->size().height());
	if(QSettings().value("window/forcerenderresolution").toBool())
	{
		renderSize.setWidth(QSettings().value("window/forcewidth").toInt());
		renderSize.setHeight(QSettings().value("window/forceheight").toInt());
	}
	return renderSize;
}

float Renderer::getAspectRatio() const
{
	QSize renderSize(getSize());
	float aspectRatio(static_cast<float>(renderSize.width())
	                  / static_cast<float>(renderSize.height()));
	return aspectRatio;
}

BasicCamera const& Renderer::getCamera(QString const& pathId) const
{
	for(auto const& pair : sceneRenderPipeline)
	{
		if(pair.first == pathId)
		{
			return *(pair.second.camera);
		}
	}
	throw(std::domain_error(std::string("Path id doesn't exist.")
	                        + pathId.toStdString()));
}

BasicCamera& Renderer::getCamera(QString const& pathId)
{
	for(auto const& pair : sceneRenderPipeline)
	{
		if(pair.first == pathId)
		{
			return *(pair.second.camera);
		}
	}
	throw(std::domain_error(std::string("Path id doesn't exist.")
	                        + pathId.toStdString()));
}

QImage Renderer::getLastFrame() const
{
	return GLHandler::generateScreenshot(
	           postProcessingTargets.at(postProcessingPipeline_.size() % 2))
	    .mirrored(false, true);
}

void Renderer::appendSceneRenderPath(QString const& id, RenderPath path)
{
	sceneRenderPipeline_.append(QPair<QString, RenderPath>(id, path));
}

void Renderer::insertSceneRenderPath(QString const& id, RenderPath path,
                                     unsigned int pos)
{
	sceneRenderPipeline_.insert(pos, QPair<QString, RenderPath>(id, path));
}

void Renderer::removeSceneRenderPath(QString const& id)
{
	for(int i(0); i < sceneRenderPipeline_.size(); ++i)
	{
		if(sceneRenderPipeline_[i].first == id)
		{
			delete sceneRenderPipeline_[i].second.camera;
			sceneRenderPipeline_.removeAt(i);
			break;
		}
	}
}

void Renderer::appendPostProcessingShader(QString const& id,
                                          QString const& fragment,
                                          QMap<QString, QString> const& defines)
{
	postProcessingPipeline_.append(QPair<QString, GLHandler::ShaderProgram>(
	    id, GLHandler::newShader("postprocess", fragment, defines)));
}

void Renderer::insertPostProcessingShader(QString const& id,
                                          QString const& fragment,
                                          unsigned int pos)
{
	postProcessingPipeline_.insert(
	    pos, QPair<QString, GLHandler::ShaderProgram>(
	             id, GLHandler::newShader("postprocess", fragment)));
}

void Renderer::removePostProcessingShader(QString const& id)
{
	for(int i(0); i < postProcessingPipeline_.size(); ++i)
	{
		if(postProcessingPipeline_[i].first == id)
		{
			postProcessingPipeline_.removeAt(i);
			break;
		}
	}
}

void Renderer::reloadPostProcessingTargets()
{
	QSize newSize(getSize());

	GLHandler::defaultRenderTargetFormat() = GL_RGBA32F;

	GLHandler::deleteRenderTarget(postProcessingTargets[0]);
	GLHandler::deleteRenderTarget(postProcessingTargets[1]);
	postProcessingTargets[0]
	    = GLHandler::newRenderTarget(newSize.width(), newSize.height());
	postProcessingTargets[1]
	    = GLHandler::newRenderTarget(newSize.width(), newSize.height());

	if(*vrHandler)
	{
		vrHandler->reloadPostProcessingTargets();
	}
}

void Renderer::renderVRControls() const
{
	if(*vrHandler)
	{
		vrHandler->renderControllers();
		vrHandler->renderHands();
	}
}

void Renderer::vrRenderSinglePath(RenderPath& renderPath, QString const& pathId,
                                  bool debug, bool debugInHeadset)
{
	GLHandler::glf().glClear(renderPath.clearMask);
	renderPath.camera->update();
	dbgCamera->update();

	if(debug && debugInHeadset)
	{
		dbgCamera->uploadMatrices();
	}
	else
	{
		renderPath.camera->uploadMatrices();
	}
	if(pathIdRenderingControllers == pathId && renderControllersBeforeScene)
	{
		renderVRControls();
	}
	// render scene
	if(wireframe)
	{
		GLHandler::beginWireframe();
	}
	window->renderScene(*renderPath.camera, pathId);
	if(pathIdRenderingControllers == pathId && !renderControllersBeforeScene)
	{
		renderVRControls();
	}
	PythonQtHandler::evalScript(
	    "if \"renderScene\" in dir():\n\trenderScene()");
	if(debug && debugInHeadset)
	{
		dbgCamera->renderCamera(renderPath.camera);
	}
	if(wireframe)
	{
		GLHandler::endWireframe();
	}
}

void Renderer::vrRender(Side side, bool debug, bool debugInHeadset)
{
	vrHandler->beginRendering(side);

	for(auto pair : sceneRenderPipeline_)
	{
		vrRenderSinglePath(pair.second, pair.first, debug, debugInHeadset);
	}

	lastFrameAverageLuminance
	    += vrHandler->getRenderTargetAverageLuminance(side);

	// do all postprocesses including last one
	int i(0);
	for(; i < postProcessingPipeline_.size(); ++i)
	{
		window->applyPostProcShaderParams(
		    postProcessingPipeline_[i].first, postProcessingPipeline_[i].second,
		    vrHandler->getPostProcessingTarget(i % 2, side));
		auto texs = window->getPostProcessingUniformTextures(
		    postProcessingPipeline_[i].first, postProcessingPipeline_[i].second,
		    vrHandler->getPostProcessingTarget(i % 2, side));
		GLHandler::postProcess(
		    postProcessingPipeline_[i].second,
		    vrHandler->getPostProcessingTarget(i % 2, side),
		    vrHandler->getPostProcessingTarget((i + 1) % 2, side), texs);
	}

	vrHandler->submitRendering(side, i % 2);
}

void Renderer::renderFrame()
{
	bool debug(dbgCamera->isEnabled());
	bool debugInHeadset(dbgCamera->debugInHeadset());
	bool renderingCamIsDebug(
	    debug && ((debugInHeadset && *vrHandler) || !(*vrHandler)));
	bool thirdRender(QSettings().value("vr/thirdrender").toBool());

	// main render logic
	if(*vrHandler)
	{
		vrHandler->prepareRendering();

		lastFrameAverageLuminance = 0.f;
		vrRender(Side::LEFT, debug, debugInHeadset);
		vrRender(Side::RIGHT, debug, debugInHeadset);
		lastFrameAverageLuminance *= 0.5f;

		if(!thirdRender && (!debug || debugInHeadset))
		{
			vrHandler->displayOnCompanion(window->size().width(),
			                              window->size().height());
		}
		else if(debug && !debugInHeadset)
		{
			renderingCamIsDebug = true;
		}
	}
	// if no VR or debug not in headset, render 2D
	if((!(*vrHandler) || thirdRender) || (debug && !debugInHeadset))
	{
		GLHandler::setClearColor(QColor(0, 0, 0, 255));
		GLHandler::beginRendering(postProcessingTargets[0]);

		for(auto pair : sceneRenderPipeline_)
		{
			GLHandler::glf().glClear(renderPath.clearMask);
			pair.second.camera->update2D();
			dbgCamera->update();
			if(renderingCamIsDebug)
			{
				dbgCamera->uploadMatrices();
			}
			else
			{
				pair.second.camera->uploadMatrices();
			}
			// render scene
			if(wireframe)
			{
				GLHandler::beginWireframe();
			}
			window->renderScene(*pair.second.camera, pair.first);
			PythonQtHandler::evalScript(
			    "if \"renderScene\" in dir():\n\trenderScene()");
			if(debug)
			{
				dbgCamera->renderCamera(pair.second.camera);
			}
			if(wireframe)
			{
				GLHandler::endWireframe();
			}
		}

		// compute average luminance
		auto tex
		    = GLHandler::getColorAttachmentTexture(postProcessingTargets[0]);
		GLHandler::generateMipmap(tex);
		unsigned int lvl = GLHandler::getHighestMipmapLevel(tex) - 3;
		auto size        = GLHandler::getTextureSize(tex, lvl);
		GLfloat* buff;
		unsigned int allocated(
		    GLHandler::getTextureContentAsData(&buff, tex, lvl));
		if(allocated > 0)
		{
			lastFrameAverageLuminance = 0.f;
			float coeffSum            = 0.f;
			float halfWidth((size.width() - 1) / 2.f);
			float halfHeight((size.height() - 1) / 2.f);
			for(int i(0); i < size.width(); ++i)
			{
				for(int j(0); j < size.height(); ++j)
				{
					unsigned int id(j * size.width() + i);
					float lum(0.2126 * buff[4 * id] + 0.7152 * buff[4 * id + 1]
					          + 0.0722 * buff[4 * id + 2]);
					float coeff
					    = exp(-1 * pow((i - halfWidth) * 2.5 / halfWidth, 2));
					coeff *= exp(-1
					             * pow((j - halfHeight) * 2.5 / halfHeight, 2));
					coeffSum += coeff;
					lastFrameAverageLuminance += coeff * lum;
				}
			}
			lastFrameAverageLuminance /= coeffSum;
			delete buff;
		}

		// postprocess
		for(int i(0); i < postProcessingPipeline_.size(); ++i)
		{
			window->applyPostProcShaderParams(postProcessingPipeline_[i].first,
			                                  postProcessingPipeline_[i].second,
			                                  postProcessingTargets.at(i % 2));
			auto texs = window->getPostProcessingUniformTextures(
			    postProcessingPipeline_[i].first,
			    postProcessingPipeline_[i].second,
			    postProcessingTargets.at(i % 2));
			GLHandler::postProcess(postProcessingPipeline_[i].second,
			                       postProcessingTargets.at(i % 2),
			                       postProcessingTargets.at((i + 1) % 2), texs);
		}
		// blit result on screen
		GLHandler::blitColorBuffer(
		    postProcessingTargets.at(postProcessingPipeline_.size() % 2),
		    GLHandler::getScreenRenderTarget());
	}
}

void Renderer::clean()
{
	if(!initialized)
	{
		return;
	}

	for(auto const& pair : sceneRenderPipeline_)
	{
		delete pair.second.camera;
	}
	for(const QPair<QString, GLHandler::ShaderProgram>& p :
	    postProcessingPipeline_)
	{
		GLHandler::deleteShader(p.second);
	}
	delete dbgCamera;

	GLHandler::deleteRenderTarget(postProcessingTargets[0]);
	GLHandler::deleteRenderTarget(postProcessingTargets[1]);

	initialized = false;
}

Renderer::~Renderer()
{
	clean();
}