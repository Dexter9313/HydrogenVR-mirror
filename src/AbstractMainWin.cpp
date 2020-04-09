#include "AbstractMainWin.hpp"

AbstractMainWin::AbstractMainWin()
{
	setSurfaceType(QSurface::OpenGLSurface);

	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	format.setVersion(4, 0);
	format.setSwapInterval(0);
	setFormat(format);

	m_context.setFormat(format);
	m_context.create();
}

bool AbstractMainWin::isFullscreen() const
{
	return QSettings().value("window/fullscreen").toBool();
}

void AbstractMainWin::setFullscreen(bool fullscreen)
{
	QSettings().setValue("window/fullscreen", fullscreen);
	if(fullscreen)
	{
		QRect screenGeometry(QApplication::desktop()->screenGeometry());
		resize(screenGeometry.width(), screenGeometry.height());
	}
	else
	{
		resize(QSettings().value("window/width").toUInt(),
		       QSettings().value("window/height").toUInt());
	}
	if(fullscreen)
	{
		showFullScreen();
	}
	else
	{
		show();
	}
}

void AbstractMainWin::reloadPythonEngine()
{
	reloadPy = true;
	PythonQtHandler::closeConsole();
}

void AbstractMainWin::toggleFullscreen()
{
	setFullscreen(!isFullscreen());
}

bool AbstractMainWin::vrIsEnabled() const
{
	return static_cast<bool>(vrHandler);
}

void AbstractMainWin::setVR(bool vr)
{
	if(vrHandler && !vr)
	{
		vrHandler.close();
	}
	else if(!vrHandler && vr)
	{
		if(vrHandler.init())
		{
			vrHandler.resetPos();
		}
	}
	if(vrIsEnabled())
	{
		PythonQtHandler::addObject("VRHandler", &vrHandler);
	}
	else
	{
		PythonQtHandler::evalScript(
		    "if \"VRHandler\" in dir():\n\tdel VRHandler");
	}
	QSettings().setValue("vr/enabled", vrIsEnabled());
}

void AbstractMainWin::toggleVR()
{
	setVR(!vrIsEnabled());
}

void AbstractMainWin::takeScreenshot(QString path) const
{
	QImage screenshot(GLHandler::generateScreenshot());
	if(path == "")
	{
		path = QFileDialog::getSaveFileName(
		    nullptr, tr("Save Screenshot"),
		    QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
		    tr("Images (*.png *.xpm *.jpg)"));
	}
	screenshot.mirrored(false, true).save(path);
}

bool AbstractMainWin::event(QEvent* e)
{
	if(e->type() == QEvent::UpdateRequest)
	{
		if(isExposed())
		{
			paintGL();
		}
		// animate continuously: schedule an update
		QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
		return true;
	}
	if(e->type() == QEvent::Type::Close)
	{
		PythonQtHandler::closeConsole();
	}
	return QWindow::event(e);
}

void AbstractMainWin::resizeEvent(QResizeEvent* /*ev*/)
{
	renderer.windowResized();
}

void AbstractMainWin::keyPressEvent(QKeyEvent* e)
{
	QString modifier;
	QString key;

	if((e->modifiers() & Qt::ShiftModifier) != 0u)
	{
		modifier += "Shift+";
	}
	if((e->modifiers() & Qt::ControlModifier) != 0u)
	{
		modifier += "Ctrl+";
	}
	if((e->modifiers() & Qt::AltModifier) != 0u)
	{
		modifier += "Alt+";
	}
	if((e->modifiers() & Qt::MetaModifier) != 0u)
	{
		modifier += "Meta+";
	}

	key = QKeySequence(e->key()).toString();

	QKeySequence ks(modifier + key);
	actionEvent(inputManager[ks], true);

	if(!PythonQtHandler::isSupported())
	{
		return;
	}

	QString pyKeyEvent("QKeyEvent(");
	pyKeyEvent += QString::number(e->type()) + ",";
	pyKeyEvent += QString::number(e->key()) + ",";
	pyKeyEvent += QString::number(e->modifiers()) + ",";
	pyKeyEvent += QString::number(e->nativeScanCode()) + ",";
	pyKeyEvent += QString::number(e->nativeVirtualKey()) + ",";
	pyKeyEvent += QString::number(e->nativeModifiers()) + ",";
	if(e->key() != Qt::Key_Return && e->key() != Qt::Key_Enter)
	{
		pyKeyEvent += "\"" + e->text().replace('"', "\\\"") + "\",";
	}
	else
	{
		pyKeyEvent += R"("\n",)";
	}
	pyKeyEvent += e->isAutoRepeat() ? "True," : "False,";
	pyKeyEvent += QString::number(e->count()) + ")";

	PythonQtHandler::evalScript(
	    "if \"keyPressEvent\" in dir():\n\tkeyPressEvent(" + pyKeyEvent + ")");
}

void AbstractMainWin::keyReleaseEvent(QKeyEvent* e)
{
	QString modifier;
	QString key;

	if((e->modifiers() & Qt::ShiftModifier) != 0u)
	{
		modifier += "Shift+";
	}
	if((e->modifiers() & Qt::ControlModifier) != 0u)
	{
		modifier += "Ctrl+";
	}
	if((e->modifiers() & Qt::AltModifier) != 0u)
	{
		modifier += "Alt+";
	}
	if((e->modifiers() & Qt::MetaModifier) != 0u)
	{
		modifier += "Meta+";
	}

	key = QKeySequence(e->key()).toString();

	QKeySequence ks(modifier + key);
	actionEvent(inputManager[ks], false);

	if(!PythonQtHandler::isSupported())
	{
		return;
	}

	QString pyKeyEvent("QKeyEvent(");
	pyKeyEvent += QString::number(e->type()) + ",";
	pyKeyEvent += QString::number(e->key()) + ",";
	pyKeyEvent += QString::number(e->modifiers()) + ",";
	pyKeyEvent += QString::number(e->nativeScanCode()) + ",";
	pyKeyEvent += QString::number(e->nativeVirtualKey()) + ",";
	pyKeyEvent += QString::number(e->nativeModifiers()) + ",";
	if(e->key() != Qt::Key_Return && e->key() != Qt::Key_Enter)
	{
		pyKeyEvent += "\"" + e->text().replace('"', "\\\"") + "\",";
	}
	else
	{
		pyKeyEvent += R"("\n",)";
	}
	pyKeyEvent += e->isAutoRepeat() ? "True," : "False,";
	pyKeyEvent += QString::number(e->count()) + ")";

	PythonQtHandler::evalScript(
	    "if \"keyReleaseEvent\" in dir():\n\tkeyReleaseEvent(" + pyKeyEvent
	    + ")");
}

void AbstractMainWin::actionEvent(BaseInputManager::Action a, bool pressed)
{
	if(!pressed)
	{
		return;
	}

	if(a.id == "toggledbgcam")
	{
		renderer.getDebugCamera().toggle();
	}
	else if(a.id == "togglewireframe")
	{
		toggleWireframe();
	}
	else if(a.id == "togglepyconsole")
	{
		PythonQtHandler::toggleConsole();
	}
	else if(a.id == "togglevr")
	{
		toggleVR();
	}
	else if(a.id == "screenshot")
	{
		takeScreenshot();
	}
	else if(a.id == "togglefullscreen")
	{
		toggleFullscreen();
	}
	else if(a.id == "quit")
	{
		close();
	}
}

void AbstractMainWin::vrEvent(VRHandler::Event const& e)
{
	PythonQtHandler::evalScript(
	    "if \"vrEvent\" in dir():\n\tvrEvent("
	    + QString::number(static_cast<int>(e.type)) + ","
	    + QString::number(static_cast<int>(e.side)) + ", "
	    + QString::number(static_cast<int>(e.button)) + ")");
}

void AbstractMainWin::setupPythonAPI()
{
	PythonQtHandler::addObject("HydrogenVR", this);
}

void AbstractMainWin::applyPostProcShaderParams(
    QString const& id, GLHandler::ShaderProgram shader,
    GLHandler::RenderTarget const& /*currentTarget*/) const
{
	if(id == "colors")
	{
		GLHandler::setShaderParam(shader, "gamma", gamma);
	}
	else
	{
		QString pyCmd("if \"applyPostProcShaderParams\" in "
		              "dir():\n\tapplyPostProcShaderParams(\""
		              + id + "\"," + QString::number(shader) + ")");
		PythonQtHandler::evalScript(pyCmd);
	}
}

std::vector<GLHandler::Texture>
    AbstractMainWin::getPostProcessingUniformTextures(
        QString const& /*id*/, GLHandler::ShaderProgram /*shader*/,
        GLHandler::RenderTarget const& /*currentTarget*/) const
{
	return {};
}

void AbstractMainWin::toggleWireframe()
{
	setWireframe(!getWireframe());
}

void AbstractMainWin::initializeGL()
{
	m_context.makeCurrent(this);
	// Init GL
	GLHandler::init();
	// Init Renderer
	renderer.init(this, &vrHandler);
	// Init PythonQt
	initializePythonQt();
	// Init VR
	setVR(QSettings().value("vr/enabled").toBool());
	// Init libraries
	initLibraries();

	// NOLINTNEXTLINE(hicpp-no-array-decay)
	qDebug() << "Using OpenGL " << format().majorVersion() << "."
	         << format().minorVersion() << '\n';

	if(vrHandler)
	{
		vrHandler.resetPos();
	}

	// let user init
	initScene();

	// Init Python engine
	setupPythonScripts();

	// make sure gamma correction is applied last
	if(QSettings().value("graphics/dithering").toBool())
	{
		renderer.appendPostProcessingShader("colors", "colors",
		                                    {{"DITHERING", "0"}});
	}
	else
	{
		renderer.appendPostProcessingShader("colors", "colors");
	}

	frameTimer.start();
	initialized = true;
}

void AbstractMainWin::initializePythonQt()
{
	PythonQtHandler::init();
	PythonQtHandler::addClass<int>("Side");
	PythonQtHandler::addObject("Side", new PySide);
	PythonQtHandler::addObject("GLHandler", new GLHandler);
}

void AbstractMainWin::reloadPythonQt()
{
	PythonQtHandler::clean();
	initializePythonQt();
	setupPythonScripts();
	reloadPy = false;
}

void AbstractMainWin::setupPythonScripts()
{
	setupPythonAPI();

	QString mainScriptPath(QSettings().value("scripting/rootdir").toString()
	                       + "/main.py");
	if(QFile(mainScriptPath).exists())
	{
		PythonQtHandler::evalFile(mainScriptPath);
	}

	PythonQtHandler::evalScript("if \"initScene\" in dir():\n\tinitScene()");
}

void AbstractMainWin::paintGL()
{
	m_context.makeCurrent(this);
	if(!initialized)
	{
		initializeGL();
	}

	frameTiming_ = frameTimer.nsecsElapsed() * 1.e-9f;
	frameTimer.restart();

	setTitle(QString(PROJECT_NAME) + " - "
	         + QString::number(round(1.f / frameTiming)) + " FPS");

	if(reloadPy)
	{
		reloadPythonQt();
	}
	if(vrHandler)
	{
		frameTiming_ = vrHandler.getFrameTiming() / 1000.f;
	}

	// handle VR events if any
	if(vrHandler)
	{
		auto e = new VRHandler::Event;
		while(vrHandler.pollEvent(e))
		{
			vrEvent(*e);
		}
		delete e;
	}
	// let user update before rendering
	for(auto const& pair : renderer.sceneRenderPipeline)
	{
		updateScene(*pair.second.camera, pair.first);
	}
	PythonQtHandler::evalScript(
	    "if \"updateScene\" in dir():\n\tupdateScene()");

	// Render frame
	renderer.renderFrame();

	// garbage collect some resources
	AsyncTexture::garbageCollect();
	AsyncMesh::garbageCollect();

	if(videomode)
	{
		QImage frame(renderer.getLastFrame());
		QString number
		    = QString("%1").arg(currentVideoFrame, 5, 10, QChar('0'));

		QThreadPool::globalInstance()->start(
		    new ImageWriter(QSettings().value("window/viddir").toString()
		                        + "/frame" + number + ".png",
		                    frame));

		currentVideoFrame++;
	}

	// Trigger a repaint immediatly
	m_context.swapBuffers(this);
}

AbstractMainWin::~AbstractMainWin()
{
	// force garbage collect some resources
	AsyncTexture::garbageCollect(true);
	AsyncMesh::garbageCollect(true);

	PythonQtHandler::evalScript(
	    "if \"cleanUpScene\" in dir():\n\tcleanUpScene()");
	renderer.clean();
	vrHandler.close();
	PythonQtHandler::clean();
}
