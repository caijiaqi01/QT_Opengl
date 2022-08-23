#ifndef OPENGLMATHMODE_H
#define OPENGLMATHMODE_H

#include <QtWidgets/QWidget>
#include <QtOpenGL/QGLWidget>
#include <QtOpenGL/QGLFunctions>
#include <QtOpenGL\QGLShader>
#include <QtOpenGL\QGLBuffer>
#include <QtOpenGL\QGLShaderProgram>
#include <QOpenGLTexture>
#include <QKeyEvent>

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
	void keyPressEvent(QKeyEvent* keyEvent);

public slots:
	void mousePressEvent(QMouseEvent*)  override;
	void mouseMoveEvent(QMouseEvent*)  override;
	void mouseReleaseEvent(QMouseEvent*)  override;

private:
	void InitShader();
	void InitBuffer();

public:
	void calculatXYZ();
	void proj();
	void setWorldGrid();

private:
	Ui::OpenglMathMode* ui;

	QGLShaderProgram shader;
	QGLBuffer* vertexBuffer; //顶点坐标
	QGLBuffer* indexBuffer; //顶点索引
	QGLBuffer* uvBuffer;   //纹理贴图缓冲区
	QOpenGLTexture* m_texture;  //纹理图片
	QMatrix4x4 matrixProjectionx;
	QMatrix4x4 matrixModelViewProjectionx;
	QMatrix4x4 matrixNormalx;
	QMatrix4x4 matrixViewx;
	int btgauche, btdroit, btmilieu, latence;

	//相机位置
	float cameraDistance = 15.4f;
	bool mouseLeftDown = false;
	bool mouseRightDown = false;
	float mouseY = 0;
	qreal acc;
	QVector3D n;
	QQuaternion rotation = QQuaternion::fromAxisAndAngle(QVector3D(1.0, 0.0, 0.0), 270) *
		QQuaternion::fromAxisAndAngle(QVector3D(0.0, 0.0, 1.0), 225) *
		QQuaternion::fromAxisAndAngle(QVector3D(1.0, -1.0, 0.0), -29);   //旋转矩阵
	QQuaternion rotationx;
	QQuaternion rotationy;
	QQuaternion rotationz;
	float translatex = 0.0f;
	float translatey = 0.0f;
	float translatez = 0.0f;
	QQuaternion oldRotation = rotation;

	//灯光
	QVector4D lightPosition = { 2.0f, 0.0f, 0.0f, 1.0f };  //光照的方向
	QVector4D lightAmbient = { 0.5f, 0.5f, 0.5f, 0.1f };//{0.4f, 0.4f, 0.4f, 0.1};
	QVector4D lightDiffuse = { 0.8f, 0.8f, 0.8f, 1.0f };
	QVector4D lightSpecular = { 0.5f, 0.5f, 0.5f, 0.1f };//{0.4f, 0.4f, 0.4f, 0.1};
	QVector4D frontColor = { 0.72f, 0.5f, 0.1f, 1.0f };
	QVector4D backColor = { 0.1f, 0.7f, 0.2f, 1.0f };
	float shininessVal = 20.0f;   //高光尖锐程度的指数值

	std::vector<float> worldGrid;    // 世界背景网格
	int PlanStartIndex = 0;      // 世界网格起始点

//顶点数组
	std::vector<float> vertices = {
		0.5f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
	   -0.5f, -0.5f, 0.0f,
	   -0.5f,  0.5f, 0.0f
	};

	//顶点索引
	std::vector<unsigned int> indices =
	{
		0, 1, 3,
		1, 2, 3
	};

	std::vector <float> uvData = {
		0.0, 0.0
	};

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
