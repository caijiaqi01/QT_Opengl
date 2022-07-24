#include "openglmathmode.h"
#include "ui_openglmathmode.h"
#include <cmath>

//gl_Position = matrixModelViewProjection * vec4(vertexPosition, 1.0);\n\
// gl_Position = vec4(aPos, 1.0);\n\
// 	
//������ɫ��
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


//vec4(r, g, b, a), ǰ����������ʾƬԪ������ɫֵRGB�����ĸ�������ƬԪ����͸����A��1.0��ʾ��͸��, 0.0��ʾ��ȫ͸����
//Ƭ����ɫ��ֻ��Ҫһ��������������������һ��4��������������ʾ�������յ������ɫ��
// ����Ӧ���Լ����������������ǿ�����out�ؼ����������������������������ΪFragColor��
// ���棬���ǽ�һ��alphaֵΪ1.0(1.0������ȫ��͸��)���ٻ�ɫ��vec4��ֵ����ɫ�����
// gl_FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n\ (Ҳ��ʹ�����ñ����޸���ɫ)
//Ƭ����ɫ��
QString fragmentShaderSource =
"#version 330 core\n\
out vec4 FragColor;\n\
void main()\n\
{\n\
	FragColor = vec4(1.0, 0.0f, 0.0f, 1.0f);\n\
}";

//��������
std::vector<float> vertices = {
	0.5f, 0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
   -0.5f, -0.5f, 0.0f,
   -0.5f,  0.5f, 0.0f
};

//��������
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
QOpenGLFunctions::initializeOpenGLFunctions()�� Ϊ��ǰ�����ĳ�ʼ��OpenGL�������������ô˺�����QOpenGLFunctions����ֻ���뵱ǰ�������Լ����乲�������������һ��ʹ�á��ٴε���initializeOpenGLFunctions()�Ը��Ķ���������Ĺ�����
QOpenGLContext::makeCurrent()�� �ڸ�����surface��ʹ��ǰ�߳��е������ĳ�Ϊ��ǰ�����ġ��ɹ�����true, ���򷵻�false���������δ��¶��������������Ӧ�ó��򱻹��������ͼ��Ӳ�������ã�����߿��ܷ�����
QOpenGLContext::swapBuffers()�� ������Ⱦ�����ǰ�󻺳��������ô����������OpenGL��Ⱦ�Ŀ�ܣ���ȷ���ڷ����κ�����OpenGL������磬��Ϊ�¿�ܵ�һ���֣�֮ǰ�ٴε���makeCurrent()��
*/

void OpenglMathMode::initializeGL()
{
	//���òü���
	glViewport(0, 0, viewPortWidth, viewPortHeight);
	//������Ȳ���
	glEnable(GL_DEPTH_TEST);

	//����ͶӰ����
	proj();

	//Ϊ��ǰ�����ĳ�ʼ��OpenGL��������
	initializeGLFunctions();

	//��ʼ����ɫ��
	InitShader();

	//��ʼ��������
	InitBuffer();
}

void OpenglMathMode::resizeGL(int w, int h)
{
	if (w > 300) {
		viewPortWidth = w;
		viewPortHeight = h;
	}

	//����ͶӰ����
	proj();
}

/*

	//��ͼ����
	QMatrix4x4 view;
	view.setToIdentity();//�Ծ�����е�λ������֤���о������㶼����һ����λ�����Ͻ���
	view.lookAt(QVector3D(2, 2, 2),     //�۾���λ��
				QVector3D(0, 0, 0),     //�۾�����λ��
				QVector3D(0, 1, 0));    //�˲�����ʾ����������۾����ϵķ���

	//ģ�;���
	QMatrix4x4 model;
	model.setToIdentity();
	model.rotate(QTime::currentTime().msec(), 1.0f, 5.0f, 0.5f);//��ת �þ���������ʸ�� (x, y, z) ��תangle��
	model.scale(0.6f);//����
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
	float Umax = 2 * PI; // x,y �������
	float Vmin = -PI;
	float Vmax = PI;     // z �������

	std::vector<float> valU;
	std::vector<float> valV;
	// �����ܶ�
	int step = 64;

	// xyz���� ���ֵ����Сֵ��
	float u_l = Umax - Umin;
	float v_l = Vmax -Vmin;

	// ������ֳ����ɶν��м���
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
			// ���㶥������
			vertices[step* 6 * i + j*6]   = Fx;
			vertices[step* 6 * i + j*6+1] = Fy;
			vertices[step* 6 * i + j*6+2] = Fz;
			vertices[step* 6 * i + j*6+3] = 0.0f;
			vertices[step* 6 * i + j*6+4] = 0.0f;
			vertices[step* 6 * i + j*6+5] = 0.0f;

			// ������������
			if (i < step - 1) {
				// ������1
				indices[step* 6 * i + j*6]   = j   + step*i;
				indices[step* 6 * i + j*6+1] = j+1 + step*i;
				indices[step* 6 * i + j*6+2] = j+1 + step*i + step;

				// ������2
				indices[step* 6 * i + j*6 + 3] = j  + step*i;
				indices[step* 6 * i + j*6 + 4] = j  + step*i+ step;
				indices[step* 6 * i + j*6 + 5] = j+1+ step*i+ step;
			}
		}
	}
}


/*
	//ͶӰ����
	QMatrix4x4 projection;
	projection.perspective(60,  //������Ұ�ĽǶ� ������ͷ������������£���Ұ���Ϊ60��
	(float)width()/height(), //���ڱ���
	0.1f,100);//���������ı�ʾ����������������Զ��Χ�ڵ�����Ҫ����ʾ�����������Χ�����彫�޷�����ʾ��

*/
void OpenglMathMode::proj()
{
	qreal aspect = qreal(viewPortWidth) / qreal(viewPortHeight ? viewPortHeight : 1);
	const qreal fov = 100.0, zNear = 0.01, zFar = 20;
	matrixProjectionx.setToIdentity();
	matrixProjectionx.perspective(fov, aspect, zNear, zFar);
}

/*
gl_Position ����û������in��out����uniform������������ֱ��ʹ�ã����ں���ĳ�����Ҳδ�����á�ԭ������Ĭ���ǹ�һ���Ĳü��ռ����꣬xyz����ά�ȵķ�ΧΪ - 1��1�������ڶ�����ɫ����ʹ�ã���������Ҳ�������gl_Position��ֵ��Χ����float��ȡֵ��Χ(32λ)��ֻ����ֻ��[-1, 1]�����ƬԪ�����ơ�����vec4���͵ģ�����������Ϊdvec4�����͡�
gl_Position����ͨ���ӽǻ���ת��Ϊ��׼���豸�ռ��еĵѿ������꣺
vec3 ndc = gl_Position.xyz / gl_Position.w;
*/

