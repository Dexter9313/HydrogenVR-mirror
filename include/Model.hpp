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

#ifndef MODEL_H
#define MODEL_H

#include <vector>

#include "AssetLoader.hpp"
#include "GLHandler.hpp"

class Model
{
  public:
	struct Light
	{
		Light()
		    : position(0.f, 0.f, 0.f)
		    , color(255, 255, 255)
		    , ambiantFactor(1.f){};
		QVector3D position;
		QColor color;
		float ambiantFactor;
	};

	// takes ownership of shader
	explicit Model(QString const& modelName, GLHandler::ShaderProgram shader
	                                         = GLHandler::newShader("model"));
	GLHandler::ShaderProgram getShader() const { return shader; };
	float getBoundingSphereRadius() { return boundingSphereRadius; };
	void render(QMatrix4x4 const& model = QMatrix4x4(),
	            Light const& light      = Light(),
	            GLHandler::GeometricSpace geometricSpace
	            = GLHandler::GeometricSpace::WORLD);
	~Model();

  private:
	GLHandler::ShaderProgram shader;
	std::vector<AssetLoader::TexturedMesh> meshes;

	float boundingSphereRadius = 0.f;
};

#endif // MODEL
