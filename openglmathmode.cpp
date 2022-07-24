#include "openglmathmode.h"
#include "ui_openglmathmode.h"
#include <cmath>

//gl_Position = matrixModelViewProjection * vec4(vertexPosition, 1.0);\n\
// gl_Position = vec4(aPos, 1.0);\n\
// 	
//顶点着色器
QString vertexShaderSource =
	"#version 330 core\n\
	layout (location = 0) in vec3 aPos;\n\
	attribute vec3 vertexPosition;\n\
    uniform mat4 matrixModelView;\n\
	uniform mat4 matrixModelViewProjection;\n\
	void main()\n\
	{\n\
		gl_Position = matrixModelViewProjection * vec4(aPos, 1.0);\n\
	}";


//vec4(r, g, b, a), 前三个参数表示片元像素颜色值RGB，第四个参数是片元像素透明度A，1.0表示不透明, 0.0表示完全透明。
//片段着色器只需要一个输出变量，这个变量是一个4分量向量，它表示的是最终的输出颜色，
// 我们应该自己将其计算出来。我们可以用out关键字声明输出变量，这里我们命名为FragColor。
// 下面，我们将一个alpha值为1.0(1.0代表完全不透明)的橘黄色的vec4赋值给颜色输出。
// gl_FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n\ (也可使用内置变量修改颜色)
//片段着色器
QString fragmentShaderSource =
"#version 330 core\n\
out vec4 FragColor;\n\
void main()\n\
{\n\
	FragColor = vec4(1.0, 0.0f, 0.0f, 1.0f);\n\
}";

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

OpenglMathMode::OpenglMathMode(QWidget* parent)
	: QGLWidget(parent)
	, viewPortWidth(1600.0)
	, viewPortHeight(1000.0)
{
}

OpenglMathMode::~OpenglMathMode()
{
}

/*
QOpenGLFunctions::initializeOpenGLFunctions()： 为当前上下文初始化OpenGL函数解析。调用此函数后，QOpenGLFunctions对象只能与当前上下文以及与其共享的其他上下文一起使用。再次调用initializeOpenGLFunctions()以更改对象的上下文关联。
QOpenGLContext::makeCurrent()： 在给定的surface上使当前线程中的上下文成为当前上下文。成功返回true, 否则返回false。如果表面未暴露，或者由于例如应用程序被挂起而导致图形硬件不可用，则后者可能发生。
QOpenGLContext::swapBuffers()： 交换渲染表面的前后缓冲区。调用此命令以完成OpenGL渲染的框架，并确保在发出任何其他OpenGL命令（例如，作为新框架的一部分）之前再次调用makeCurrent()。
*/

void OpenglMathMode::initializeGL()
{
	//设置裁剪面
	glViewport(0, 0, viewPortWidth, viewPortHeight);
	//开启深度测试
	glEnable(GL_DEPTH_TEST);

	//计算投影矩阵
	proj();

	//为当前上下文初始化OpenGL函数解析
	initializeGLFunctions();

	//初始化着色器
	InitShader();

	//初始化缓冲区
	InitBuffer();
}

void OpenglMathMode::resizeGL(int w, int h)
{
	if (w > 300) {
		viewPortWidth = w;
		viewPortHeight = h;
	}

	//计算投影矩阵
	proj();
}

/*

	//视图矩阵
	QMatrix4x4 view;
	view.setToIdentity();//对矩阵进行单位化，保证所有矩阵运算都会在一个单位矩阵上进行
	view.lookAt(QVector3D(2, 2, 2),     //眼睛的位置
				QVector3D(0, 0, 0),     //眼睛看的位置
				QVector3D(0, 1, 0));    //此参数表示方向，相对于眼睛向上的方向。

	//模型矩阵
	QMatrix4x4 model;
	model.setToIdentity();
	model.rotate(QTime::currentTime().msec(), 1.0f, 5.0f, 0.5f);//旋转 该矩阵将坐标绕矢量 (x, y, z) 旋转angle度
	model.scale(0.6f);//缩放
*/

void OpenglMathMode::paintGL()
{
	resize(viewPortWidth, viewPortHeight);
	glViewport(0, 0, viewPortWidth, viewPortHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set modelview matrix
	matrixViewx.setToIdentity();
	matrixViewx.translate(0.0, 0.0, -cameraDistance);
	matrixViewx.rotate(rotation);
	matrixModelViewProjectionx = matrixProjectionx * matrixViewx;
	matrixNormalx = matrixViewx;
	matrixNormalx.setColumn(3, QVector4D(0, 0, 0, 1));

	indexBuffer->bind();
	shader.bind();
	glUniformMatrix4fv(uniformMatrixModelView, 1, false, matrixViewx.data());
	glUniformMatrix4fv(uniformMatrixModelViewProjection, 1, false, matrixModelViewProjectionx.data());
	//glUniformMatrix4fv(uniformMatrixNormal, 1, false, matrixNormalx.data());
	glEnableVertexAttribArray(attribVertexPosition);
	//// set attrib arrays using glVertexAttribPointer()
	glVertexAttribPointer(attribVertexPosition, 3, GL_FLOAT, false, 3 * sizeof(GL_FLOAT), (void*)0);

	shader.enableAttributeArray("aPos");
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	shader.disableAttributeArray("aPos");
	shader.release();
	indexBuffer->release();
}

void OpenglMathMode::InitShader()
{
	shader.addShaderFromSourceCode(QGLShader::Vertex, vertexShaderSource);
	shader.addShaderFromSourceCode(QGLShader::Fragment, fragmentShaderSource);
	shader.link();
	shaderprogramId = shader.programId();

	uniformMatrixModelView = glGetUniformLocation(shaderprogramId, "matrixModelView");
	uniformMatrixModelViewProjection = glGetUniformLocation(shaderprogramId, "matrixModelViewProjection");
	attribVertexPosition = glGetAttribLocation(shaderprogramId, "vertexPosition");
}

void OpenglMathMode::InitBuffer()
{
	vertexBuffer = new QGLBuffer(QGLBuffer::VertexBuffer);
	indexBuffer = new QGLBuffer(QGLBuffer::IndexBuffer);

	calculatXYZ();

	shader.bind();
	vertexBuffer->create();
	vertexBuffer->bind();
	vertexBuffer->allocate(&vertices[0], sizeof(GL_FLOAT) * vertices.size());
	vertexBuffer->setUsagePattern(QGLBuffer::StaticDraw);
	shader.setAttributeBuffer("aPos", GL_FLOAT, 0, 3, 6 * sizeof(GL_FLOAT));
	vertexBuffer->release();
	shader.release();

	indexBuffer->create();
	indexBuffer->bind();
	indexBuffer->allocate(&indices[0], sizeof(int) * indices.size());
	indexBuffer->setUsagePattern(QGLBuffer::StaticDraw);
	indexBuffer->release();
}


void OpenglMathMode::calculatXYZ() 
{
	float Umin = 0.0f;
	float Umax = 2 * PI; // x,y 坐标参数
	float Vmin = -PI;
	float Vmax = PI;     // z 坐标参数

	std::vector<float> valU;
	std::vector<float> valV;
	// 网格密度
	int step = 64;

	// xyz坐标 最大值和最小值差
	float u_l = Umax - Umin;
	float v_l = Vmax -Vmin;

	// 将坐标分成若干段进行计算
	for (unsigned int i = 0; i < step; i++) {
		float u = (i * u_l) / (step - 1) + Umin;
		valU.push_back(u);

		float v = (i * v_l) / (step - 1) + Vmin;
		valV.push_back(v);
	}

	vertices.resize(step * step * 6);
	indices.resize(step * (step-1) * 6);

	for (int i = 0; i < step; i++) {
		for (int j = 0; j < step; j++) {
			float Fx = cos(valU[j]) * (4 + (19 / 5) * cos(valV[i]));
			float Fy = sin(valU[j]) * (4 + (19 / 5) * cos(valV[i]));
			float Fz = (cos(valV[i]) + sin(valV[i]) - 1) * (1 + sin(valV[i])) * log(1 - PI * valV[i] / 10) + (15 / 2) * sin(valV[i]);
			// 计算顶点坐标
			vertices[step* 6 * i + j*6]   = Fx;
			vertices[step* 6 * i + j*6+1] = Fy;
			vertices[step* 6 * i + j*6+2] = Fz;
			vertices[step* 6 * i + j*6+3] = 0.0f;
			vertices[step* 6 * i + j*6+4] = 0.0f;
			vertices[step* 6 * i + j*6+5] = 0.0f;

			// 计算三角网格
			if (i < step - 1) {
				// 三角形1
				indices[step* 6 * i + j*6]   = j   + step*i;
				indices[step* 6 * i + j*6+1] = j+1 + step*i;
				indices[step* 6 * i + j*6+2] = j+1 + step*i + step;

				// 三角形2
				indices[step* 6 * i + j*6 + 3] = j  + step*i;
				indices[step* 6 * i + j*6 + 4] = j  + step*i+ step;
				indices[step* 6 * i + j*6 + 5] = j+1+ step*i+ step;
			}
		}
	}
}


/*
	//投影矩阵
	QMatrix4x4 projection;
	projection.perspective(60,  //画面视野的角度 人眼在头部不动的情况下，视野大概为60°
	(float)width()/height(), //窗口比例
	0.1f,100);//参数三和四表示距离摄像机多近个多远范围内的物体要被显示。超出这个范围的物体将无法被显示。

*/
void OpenglMathMode::proj()
{
	qreal aspect = qreal(viewPortWidth) / qreal(viewPortHeight ? viewPortHeight : 1);
	const qreal fov = 100.0, zNear = 0.01, zFar = 20;
	matrixProjectionx.setToIdentity();
	matrixProjectionx.perspective(fov, aspect, zNear, zFar);
}

/*
gl_Position 它并没有类型in、out或是uniform的声明，而是直接使用，且在后面的程序中也未被引用。原来它是默认是归一化的裁剪空间坐标，xyz各个维度的范围为 - 1到1，仅能在顶点着色器中使用，既是输入也是输出。gl_Position赋值范围就是float的取值范围(32位)，只不过只有[-1, 1]区间的片元被绘制。它是vec4类型的，不能重声明为dvec4等类型。
gl_Position可以通过视角划分转换为标准化设备空间中的笛卡尔坐标：
vec3 ndc = gl_Position.xyz / gl_Position.w;
*/

