#ifndef OPENGLMATHMODE_H
#define OPENGLMATHMODE_H

#include <QtWidgets/QWidget>
#include <QtOpenGL/QGLWidget>
#include <QtOpenGL/QGLFunctions>
#include <QtOpenGL\QGLShader>
#include <QtOpenGL\QGLBuffer>
#include <QtOpenGL\QGLShaderProgram>

#define PI (double(314159265) / double(100000000))

namespace Ui {
class OpenglMathMode;
}

class OpenglMathMode : public QGLWidget, protected QGLFunctions
{
    Q_OBJECT

public:
    explicit OpenglMathMode(QWidget *parent = 0);
    ~OpenglMathMode();

protected:
	void initializeGL();             //初始化OpenGL
	void resizeGL(int w, int h);     //调整oeenGL的显示窗口
	void paintGL();                  //绘制opengl图像

private:
	void InitShader();
	void InitBuffer();

public:
	void calculatXYZ();
	void proj();

private:
	Ui::OpenglMathMode* ui;

	QGLShaderProgram shader;
	QGLBuffer* vertexBuffer;
	QGLBuffer* indexBuffer;
	QMatrix4x4 matrixProjectionx;
	QMatrix4x4 matrixModelViewProjectionx;
	QMatrix4x4 matrixNormalx;
	QMatrix4x4 matrixViewx;

	float cameraDistance = 11.4f;       //相机位置
	QQuaternion rotation = QQuaternion::fromAxisAndAngle(QVector3D(1.0, 0.0, 0.0), 270) *
		QQuaternion::fromAxisAndAngle(QVector3D(0.0, 0.0, 1.0), 225) *
		QQuaternion::fromAxisAndAngle(QVector3D(1.0, -1.0, 0.0), -29);   //旋转矩阵
	QQuaternion rotationx;
	QQuaternion rotationy;
	QQuaternion rotationz;
	QQuaternion oldRotation = rotation;

	GLint uniformFrontColor;
	GLint uniformBackColor;
	GLint uniformGridColor;
	GLint uniformThereisRGBA;
	GLint uniformHSVactive;
	GLint uniformShininess;
	GLint uniformglFrontFacing_1;
	GLint uniformglFrontFacing_2;
	GLint uniformdrawgridColor;
	GLint uniformMatrixModelView;
	GLint uniformMatrixModelViewProjection;
	GLint uniformMatrixNormal;
	GLint uniformLightPosition;
	GLint uniformLightAmbient;
	GLint uniformLightDiffuse;
	GLint uniformLightSpecular;
	GLint attribVertexPosition;
	GLint attribVertexNormal;
	GLint attribVertexColor;
	GLint attribVertexTexCoord;
	QVector2D mousePressPosition;
	GLuint shaderprogramId = 0;


	float viewPortWidth;
	float viewPortHeight;


};

#endif // OPENGLMATHMODE_H
