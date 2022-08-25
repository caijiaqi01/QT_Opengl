#include "openglmathmode.h"
#include "ui_openglmathmode.h"
#include<sstream>
#include <cmath>

//gl_Position = matrixModelViewProjection * vec4(vertexPosition, 1.0);\n\
// gl_Position = vec4(aPos, 1.0);\n\
// 	
//顶点着色器
QString vertexShaderSource =
R"(#version 330 core
	layout (location = 0) in vec3 aPos;
    // varyings (output) 输出给片段着色器
	varying vec3 esVertex, esNormal;
	varying vec2 uv;
	varying vec4 color;
	varying vec4 v_position;
	attribute vec3 vertexPosition;
    attribute vec3 vertexNormal;
	attribute vec2 uvVertex;
	attribute vec4 vertexColor;
	uniform mat4 matrixNormal;
    uniform mat4 matrixModelView;
	uniform mat4 matrixModelViewProjection;
	void main()
	{
        esVertex = vec3(matrixModelView * vec4(aPos, 1.0));
		esNormal = vec3(matrixNormal * vec4(vertexNormal, 1.0));
		//color = vertexColor;
		color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
		v_position = -matrixModelView * vec4(aPos, 1.0);
		gl_Position = matrixModelViewProjection * vec4(aPos, 1.0);
		uv = uvVertex;
	})";


//vec4(r, g, b, a), 前三个参数表示片元像素颜色值RGB，第四个参数是片元像素透明度A，1.0表示不透明, 0.0表示完全透明。
//片段着色器只需要一个输出变量，这个变量是一个4分量向量，它表示的是最终的输出颜色，
// 我们应该自己将其计算出来。我们可以用out关键字声明输出变量，这里我们命名为FragColor。
// 下面，我们将一个alpha值为1.0(1.0代表完全不透明)的橘黄色的vec4赋值给颜色输出。
// gl_FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n\ (也可使用内置变量修改颜色)
//片段着色器
QString fragmentShaderSource =
R"(#version 330 core
out vec4 FragColor;
#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif
// uniforms
uniform vec4 backColor;
uniform vec4 lightPosition;
uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform int hsvactive;
uniform int thereisRGBA;
uniform float shininess;
uniform int glFrontFacing_1;
// varyings
varying vec3 esVertex, esNormal;
varying vec2 uv;
varying vec4 color;
varying vec4 v_position;
// All components are in the range [0…1], including hue.
uniform sampler2D imgTexture;
vec3 hsv2rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
void main()
{
	vec4 color1=color;
	vec3 normal = normalize(esNormal);
	vec3 light;
	if (lightPosition.w == 0.0)
	{
		light = normalize(-lightPosition.xyz);
	}
	else
	{
		light = normalize(lightPosition.xyz - esVertex);
	}
	if (hsvactive == 1)
		color1 = vec4(hsv2rgb(color1.xyz), color1.w);
	vec3 view = normalize(-esVertex);
	vec3 halfv = normalize(light + view);
	vec4 fragColor = lightAmbient * color1;
	float dotNL = max(dot(normal, light), 0.0f);
	fragColor += lightDiffuse * dotNL * color1; // add diffuse
	float dotNH = max(dot(normal, halfv), 0.0);
	vec3 LightReflect = normalize(reflect(halfv, normal));
	float SpecularFactor = dot(vec3(0.0f, 0.0f, 1.0f), -LightReflect);
	if (SpecularFactor > 0) {
		SpecularFactor = pow(SpecularFactor, shininess);
		fragColor += lightSpecular * 1.0f * SpecularFactor;
	}
	if (glFrontFacing_1 == 1){
		FragColor = vec4(1.0, 0.0f, 0.0f, 1.0f);
	}
	else if (glFrontFacing_1 == 2) {
		FragColor = vec4(0.6f, 0.6f, 0.6f, 1.0f);
	}
	else if (glFrontFacing_1 == 3) {
		FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (glFrontFacing_1 == 4) {
		FragColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	}
	else if (glFrontFacing_1 == 5) {
		FragColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	}
	else {
		float back = dot(vec3(0.0f, 0.0f, 1.0f), normal);
		if (back < 0.0f) {
			FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
		}
		else {
			//FragColor = texture(imgTexture, uv);
			FragColor = fragColor;
		}
	}
})";

OpenglMathMode::OpenglMathMode(QWidget* parent)
	: QGLWidget(parent)
	, viewPortWidth(1600.0)
	, viewPortHeight(1000.0)
{
	m_texture = nullptr;
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
	m_texture = new QOpenGLTexture(QImage("D:/5.png")); //5.jpg

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
	matrixViewx.translate(camera_pos);
	matrixViewx.rotate(rotation);
	matrixViewx.translate(translate.x(), translate.y(), translate.z());
	matrixModelViewProjectionx = matrixProjectionx * matrixViewx;
	matrixNormalx = matrixViewx;
	matrixNormalx.setColumn(3, QVector4D(0, 0, 0, 1));

	indexBuffer->bind();
	uvBuffer->bind();
	m_texture->bind();
	shader.bind();
	glUniformMatrix4fv(uniformMatrixModelView, 1, false, matrixViewx.data());
	glUniformMatrix4fv(uniformMatrixModelViewProjection, 1, false, matrixModelViewProjectionx.data());
	glUniformMatrix4fv(uniformMatrixNormal, 1, false, matrixNormalx.data());
	//glEnableVertexAttribArray(attribVertexPosition);
	//// set attrib arrays using glVertexAttribPointer()
	//glVertexAttribPointer(attribVertexPosition, 3, GL_FLOAT, false, 3 * sizeof(GL_FLOAT), (void*)0);

	shader.enableAttributeArray("uvVertex");
	shader.enableAttributeArray("vertexNormal");
	shader.enableAttributeArray("aPos");
	shader.setUniformValue("glFrontFacing_1", 0);

	// 抗锯齿
	//glEnable(GL_LINE_SMOOTH);
	//glHint(GL_LINE_SMOOTH, GL_NICEST);

	// 多边形偏移
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	// 模型主体绘制
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glDisable(GL_POLYGON_OFFSET_FILL);

	shader.setUniformValue("glFrontFacing_1", 1);
	// 设置网格线的宽度 
	glLineWidth(0.4f);
	// 模型网格绘制
	glDrawElements(GL_LINE_LOOP, indices.size(), GL_UNSIGNED_INT, 0);

	//世界网格绘制
	shader.setUniformValue("glFrontFacing_1", 2);
	glLineWidth(0.3);
	glDrawArrays(GL_LINES, PlanStartIndex, 80);

	//X轴绘制
	shader.setUniformValue("glFrontFacing_1", 3);
	glLineWidth(3.0);
	glDrawArrays(GL_LINES, PlanStartIndex + 80, 2);

	//Y轴绘制
	shader.setUniformValue("glFrontFacing_1", 4);
	glDrawArrays(GL_LINES, PlanStartIndex + 82, 2);

	//Z轴绘制
	shader.setUniformValue("glFrontFacing_1", 5);
	glDrawArrays(GL_LINES, PlanStartIndex + 84, 2);

	shader.disableAttributeArray("aPos");
	shader.disableAttributeArray("vertexNormal");
	shader.disableAttributeArray("uvVertex");
	shader.release();
	indexBuffer->release();
	uvBuffer->release();
	m_texture->release();
}

void OpenglMathMode::InitShader()
{
	shader.addShaderFromSourceCode(QGLShader::Vertex, vertexShaderSource);
	shader.addShaderFromSourceCode(QGLShader::Fragment, fragmentShaderSource);
	shader.link();
	shaderprogramId = shader.programId();

	uniformMatrixModelView = glGetUniformLocation(shaderprogramId, "matrixModelView");
	uniformMatrixModelViewProjection = glGetUniformLocation(shaderprogramId, "matrixModelViewProjection");
	uniformMatrixNormal = glGetUniformLocation(shaderprogramId, "matrixNormal");
	attribVertexPosition = glGetAttribLocation(shaderprogramId, "vertexPosition");
}

void OpenglMathMode::InitBuffer()
{
	vertexBuffer = new QGLBuffer(QGLBuffer::VertexBuffer);
	indexBuffer = new QGLBuffer(QGLBuffer::IndexBuffer);
	uvBuffer = new QGLBuffer(QGLBuffer::VertexBuffer);

	calculatXYZ();

	//计算世界网格坐标
	setWorldGrid();

	// 顶点数组缓冲区绑定
	shader.bind();
	vertexBuffer->create();
	vertexBuffer->bind();
	vertexBuffer->allocate(&vertices[0], sizeof(GL_FLOAT) * vertices.size());
	vertexBuffer->setUsagePattern(QGLBuffer::StaticDraw);

	shader.setAttributeBuffer("aPos", GL_FLOAT, 0, 3, 6 * sizeof(GL_FLOAT));
	shader.setAttributeBuffer("vertexNormal", GL_FLOAT, 3 * sizeof(GL_FLOAT), 3, 6 * sizeof(GL_FLOAT));
	shader.setUniformValue("lightPosition", lightPosition);
	shader.setUniformValue("lightAmbient", lightAmbient);
	shader.setUniformValue("lightDiffuse", lightDiffuse);
	shader.setUniformValue("lightSpecular", lightSpecular);
	shader.setUniformValue("backColor", backColor);
	shader.setUniformValue("shininess", shininessVal);
	shader.setUniformValue("hsvactive", 0);
	shader.setUniformValue("thereisRGBA", 0);
	vertexBuffer->release();
	shader.release();

	// 纹理缓冲区绑定
	uvBuffer->create();
	uvBuffer->bind();
	uvBuffer->allocate(&uvData[0], sizeof(float) * uvData.size());
	uvBuffer->setUsagePattern(QGLBuffer::StaticDraw);
	shader.setAttributeBuffer("uvVertex", GL_FLOAT, 2 * sizeof(GL_FLOAT), 2, 2 * sizeof(GL_FLOAT));
	uvBuffer->release();

	// 顶点索引缓冲区绑定
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
	float v_l = Vmax - Vmin;

	// 将坐标分成若干段进行计算
	float one_u = u_l / (step - 1);
	float one_v = v_l / (step - 1);
	for (unsigned int i = 0; i < step; i++) {
		float u = Umax - i * one_u;
		valU.push_back(u);

		float v = Vmax - i * one_v;
		valV.push_back(v);
	}

	vertices.resize(step * step * 6);
	PlanStartIndex = vertices.size() / 6;
	indices.resize(step * (step - 1) * 6);
	uvData.resize(step * (step - 1) * 2);

	for (int i = 0; i < step; i++) {
		for (int j = 0; j < step; j++) {
			float Fx = cos(valU[j]) * (4 + (19 / 5) * cos(valV[i]));
			float Fy = sin(valU[j]) * (4 + (19 / 5) * cos(valV[i]));
			float Fz = (cos(valV[i]) + sin(valV[i]) - 1) * (1 + sin(valV[i])) * log(1 - PI * valV[i] / 10) + (15 / 2) * sin(valV[i]);
			// 计算顶点坐标
			vertices[step * 6 * i + j * 6] = Fx;
			vertices[step * 6 * i + j * 6 + 1] = Fy;
			vertices[step * 6 * i + j * 6 + 2] = Fz;
			vertices[step * 6 * i + j * 6 + 3] = 0.0f;
			vertices[step * 6 * i + j * 6 + 4] = 0.0f;
			vertices[step * 6 * i + j * 6 + 5] = 0.0f;

			// 计算三角网格
			if (i < step - 1) {
				// D C
				// A B 
				// 三角形1
				indices[step * 6 * i + j * 6] = j + step * i;        // A
				indices[step * 6 * i + j * 6 + 1] = (j + 1) % step + step * i; // B
				indices[step * 6 * i + j * 6 + 2] = (j + 1) % step + step * i + step; // C

				// 三角形2
				indices[step * 6 * i + j * 6 + 3] = j + step * i; // A
				indices[step * 6 * i + j * 6 + 4] = j + step * i + step; // D
				indices[step * 6 * i + j * 6 + 5] = (j + 1) % step + step * i + step; // C

				// 纹理坐标
				uvData[step * 2 * i + j * 2] = j * 1.0f / step;			//A.x
				uvData[step * 2 * i + j * 2 + 1] = i * 1.0f / step;		//A.y
			}
		}
	}

	uint  i, j, deplacement = 6 * step;
	float caa, bab, cab, baa, ba, ca, b4;
	for (i = 0; i+1 < step; i++) {
		for (j = 0; j+1 < step; j++) {
			/*
				A

				B    C
				右手定则得出法向量
			    AB = A - B
				BC = B - C	
			*/
			caa = vertices[(i + 1) * deplacement + j * 6 + 1] - vertices[i * deplacement + j * 6 + 1]; //y1    A.y - B.y
			bab = vertices[i * deplacement + j * 6 + 2] - vertices[i * deplacement + (j + 1) * 6 + 2];  //z2   B.z - C.z
			cab = vertices[(i + 1) * deplacement + j * 6 + 2] - vertices[i * deplacement + j * 6 + 2];  //z1   A.z - B.z
			baa = vertices[i * deplacement + j * 6 + 1] - vertices[i * deplacement + (j + 1) * 6 + 1];  //y2   B.y - C.y
			ba = vertices[i * deplacement + j * 6 + 0] - vertices[i * deplacement + (j + 1) * 6 + 0];   //x2   B.x - C.x
			ca = vertices[(i + 1) * deplacement + j * 6 + 0] - vertices[i * deplacement + j * 6 + 0];   //x1   A.x - B.x

			vertices[i * deplacement + j * 6 + 3] = caa * bab - cab * baa;
			vertices[i * deplacement + j * 6 + 4] = cab * ba - ca * bab;
			vertices[i * deplacement + j * 6 + 5] = ca * baa - caa * ba;

			//保证深度方向向外    法线计算有错误，方向反了
			if (vertices[i * deplacement + j * 6 + 5] < 0)
			{
				//vertices[i * deplacement + j * 6 + 3] = -vertices[i * deplacement + j * 6 + 3];
				//vertices[i * deplacement + j * 6 + 4] = -vertices[i * deplacement + j * 6 + 4];
				//vertices[i * deplacement + j * 6 + 5] = -vertices[i * deplacement + j * 6 + 5];
			}

			b4 = sqrt((vertices[i * deplacement + j * 6 + 3] * vertices[i * deplacement + j * 6 + 3]) +
				(vertices[i * deplacement + j * 6 + 4] * vertices[i * deplacement + j * 6 + 4]) +
				(vertices[i * deplacement + j * 6 + 5] * vertices[i * deplacement + j * 6 + 5]));
			if (b4 < float(0.000001))  b4 = float(0.000001);
			//Normalise:
			vertices[i * deplacement + j * 6 + 3] /= b4;
			vertices[i * deplacement + j * 6 + 4] /= b4;
			vertices[i * deplacement + j * 6 + 5] /= b4;
/*
			std::ostringstream oss;
			oss << vertices[i * deplacement + j * 6 + 3];
			std::string str(oss.str());
			normal1.push_back("x:  " + str);

			std::ostringstream oss1;
			oss1 << vertices[i * deplacement + j * 6 + 4];
			str = oss1.str();
			normal1.push_back("y:  " + str);

			std::ostringstream oss2;
			oss2 << vertices[i * deplacement + j * 6 + 5];
			str = oss2.str();
			normal1.push_back("z:  " + str);
			*/
		}
	}

	int a = 0;
}

void OpenglMathMode::setWorldGrid() 
{
	float step = 1.0f;
	float z = -2.5 * PI;
	for (int i = 0; i < 20; i++) {
		vertices.push_back(i * step - 10 * step); // x
		vertices.push_back(-11 * step ); // y
		vertices.push_back(z); // z

		vertices.push_back(i * step - 10 * step); // x
		vertices.push_back(-11 * step - step); // y
		vertices.push_back(z + 1.0f); // z

		vertices.push_back(i * step - 10 * step); // x
		vertices.push_back(10 * step); // y
		vertices.push_back(z); // z

		vertices.push_back(i * step - 10 * step); // x
		vertices.push_back(10 * step); // y
		vertices.push_back(z + 1.0f); // z


		vertices.push_back(-11 * step); // x
		vertices.push_back(i * step - 10 * step); // y
		vertices.push_back(z); // z

		vertices.push_back(-11 * step); // x
		vertices.push_back(i * step - 10 * step); // y
		vertices.push_back(z + 1.0f); // z

		vertices.push_back(10 * step); // x
		vertices.push_back(i * step - 10 * step); // y
		vertices.push_back(z); // z

		vertices.push_back(10 * step); // x
		vertices.push_back(i * step - 10 * step); // y
		vertices.push_back(z + 1.0f); // z
	}

	// X轴起点
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);

	// X轴起点 法向量
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(1.0f);

	// X轴终点
	vertices.push_back(10.0f);
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);

	// X轴终点 法向量
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(1.0f);

	// Y轴
	// 起点
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);

	// Y轴起点 法向量
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(1.0f);

	vertices.push_back(0.0f);
	vertices.push_back(10.0f);
	vertices.push_back(0.0f);

	// Y轴终点 法向量
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(1.0f);

	// Z轴起点
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);

	// Z轴起点 法向量
	vertices.push_back(1.0f);
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);

	// Z轴终点
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(10.0f);

	// Z轴起点 法向量
	vertices.push_back(1.0f);
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
}

/*
	//投影矩阵
	QMatrix4x4 projection;
	projection.perspective(60,  //画面视野的角度 人眼在头部不动的情况下，视野大概为60°
	(float)width()/height(), //窗口比例
	0.1f,100);//参数三和四表示距离摄像机多近个多远范围内的物体要被显示。超出这个范围的物体将无法被显示。

LookAt函数将顶点的世界空间坐标转换为观察空间坐标，实际上是以相机为原点重新定义的三维空间。
glm::mat4 LookAt(glm::vec3 pos, glm::vec3 target, glm::vec3 up)
函数接受三个参数，相机位置pos，目标位置target以及相机上向量up。工作流程是：glm::vec3 zaxis = normalize(pos-target)指向观察空间正Z轴方向，glm::vec3 xaxis = normalize(glm::cross(up, zaxis))指向观察空间正X轴方向，glm::vec3 = glm::cross(zaxis, xaxis)指向观察空间正Y轴方向。
向上的向量和视线的向量（z轴）确定一个平面，y轴一定在这个平面上。通过这个平面确定左右方向（x轴）。有了z轴、x轴，y轴就确定了。
走出去，看看月亮。地平线在底部。将头部向左倾斜，地平线向右旋转。向上矢量是你头部的方向
是。它是任意的，它可以使相机“滚动”，即看起来好像场景围绕眼轴旋转。
*/
void OpenglMathMode::proj()
{
	qreal aspect = qreal(viewPortWidth) / qreal(viewPortHeight ? viewPortHeight : 1);
	const qreal fov = 100.0, zNear = 0.01, zFar = 25;
	matrixProjectionx.setToIdentity();
	matrixProjectionx.perspective(fov, aspect, zNear, zFar);
	QVector3D center(0.0, 0.0, 0.0);
	center.setY(0);
//	matrixProjectionx.lookAt(camera_pos, center, QVector3D(0.0f, 1.0f, 0.0f));
}


void OpenglMathMode::keyPressEvent(QKeyEvent* keyEvent) {
	switch (keyEvent->key()) {
	case Qt::Key_Right:
		translate.setX(translate.x() - 0.1f);
		break;
	case Qt::Key_Left:
		translate.setX(translate.x() + 0.1f);
		break;
	case Qt::Key_Up:
		translate.setY(translate.y() - 0.1f);
		break;
	case Qt::Key_Down:
		translate.setY(translate.y() + 0.1f);
		break;
	case Qt::Key_W:
		translate.setZ(translate.z() + 0.1f);
		break;
	case Qt::Key_S:
		translate.setZ(translate.z() - 0.1f);
		break;
	}
	update();
}

void OpenglMathMode::mouseMoveEvent(QMouseEvent* e)
{
	static int oldx = 0, oldy = 0;
	static QVector3D oldn = QVector3D(0, 0, 1);
	if (mouseLeftDown)
	{
		QVector2D diff = QVector2D(e->pos()) - mousePressPosition;
		// Rotation axis is perpendicular to the mouse position difference
		n = QVector3D(diff.y(), diff.x(), 0.0).normalized();
		// Accelerate angular speed relative to the length of the mouse sweep
		acc = std::sqrt((diff.y() - oldy) * (diff.y() - oldy) + float(diff.x() - oldx) * (diff.x() - oldx)) / 3.0;
		// Calculate new rotation axis
		rotation = QQuaternion::fromAxisAndAngle(n, acc) * oldRotation;
		oldn = n;
	}
	if (mouseRightDown)
	{
		translate.setZ(translate.z() - (e->pos().y() / 2 - mouseY) * 0.02f);
		mouseY = e->pos().y() / 2;
	}
	update();
}

void OpenglMathMode::mouseReleaseEvent(QMouseEvent*)
{
	oldRotation = rotation;
}

void OpenglMathMode::mousePressEvent(QMouseEvent* e)
{
	// Save mouse press position
	mousePressPosition = QVector2D(e->pos());
	rotation = oldRotation;
	if (e->button() == Qt::LeftButton)
	{
		btgauche = 1;
		mouseLeftDown = true;
	}
	else
	{
		btgauche = 0;
		mouseLeftDown = false;
	}
	if (e->button() == Qt::RightButton)
	{
		btdroit = 1;
		mouseRightDown = true;
	}
	else
	{
		btdroit = 0;
		mouseRightDown = false;
	}
	if (e->button() == Qt::MiddleButton)
		btmilieu = 1;
	else
		btmilieu = 0;

	mouseY = e->pos().y() / 2;
}

/*
gl_Position 它并没有类型in、out或是uniform的声明，而是直接使用，且在后面的程序中也未被引用。原来它是默认是归一化的裁剪空间坐标，xyz各个维度的范围为 - 1到1，仅能在顶点着色器中使用，既是输入也是输出。gl_Position赋值范围就是float的取值范围(32位)，只不过只有[-1, 1]区间的片元被绘制。它是vec4类型的，不能重声明为dvec4等类型。
gl_Position可以通过视角划分转换为标准化设备空间中的笛卡尔坐标：
vec3 ndc = gl_Position.xyz / gl_Position.w;
*/