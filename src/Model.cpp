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

#include "Model.hpp"

Model::Model(QString const& modelName, GLHandler::ShaderProgram shader)
    : shader(shader)
{
	boundingSphereRadius
	    = AssetLoader::loadModel(modelName, meshes, textures, shader);
}

void Model::render(QMatrix4x4 const& model,
                   GLHandler::GeometricSpace geometricSpace)
{
	GLHandler::setUpRender(shader, model, geometricSpace);
	for(unsigned int i(0); i < meshes.size(); ++i)
	{
		GLHandler::useTextures({textures[i]});
		GLHandler::render(meshes[i]);
	}
}

Model::~Model()
{
	for(auto mesh : meshes)
	{
		GLHandler::deleteMesh(mesh);
	}
	for(auto tex : textures)
	{
		GLHandler::deleteTexture(tex);
	}
	GLHandler::deleteShader(shader);
}
