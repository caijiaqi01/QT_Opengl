#include "openglmathmode.h"
#include "ui_openglmathmode.h"
#include <gl/GLU.h>
#include<sstream>
#include <cmath>

//gl_Position = matrixModelViewProjection * vec4(vertexPosition, 1.0);\n\
// gl_Position = vec4(aPos, 1.0);\n\
// 	
//������ɫ��
QString vertexShaderSource =
R"(#version 330 core
	layout (location = 0) in vec3 aPos;
    // varyings (output) �����Ƭ����ɫ��
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


//vec4(r, g, b, a), ǰ����������ʾƬԪ������ɫֵRGB�����ĸ�������ƬԪ����͸����A��1.0��ʾ��͸��, 0.0��ʾ��ȫ͸����
//Ƭ����ɫ��ֻ��Ҫһ��������������������һ��4��������������ʾ�������յ������ɫ��
// ����Ӧ���Լ����������������ǿ�����out�ؼ����������������������������ΪFragColor��
// ���棬���ǽ�һ��alphaֵΪ1.0(1.0������ȫ��͸��)���ٻ�ɫ��vec4��ֵ����ɫ�����
// gl_FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n\ (Ҳ��ʹ�����ñ����޸���ɫ)
//Ƭ����ɫ��
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
// All components are in the range [0��1], including hue.
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
	else if (glFrontFacing_1 == 6) {
		FragColor = texture(imgTexture, uv);
	}
	else {
		float back = dot(vec3(0.0f, 0.0f, 1.0f), normal);
		if (back < 0.0f) {
			FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
		}
		else {
			if (glFrontFacing_1 == 6) {
				FragColor = texture(imgTexture, uv);
			}
			else {
				FragColor = fragColor;
			}
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
	m_texture = new QOpenGLTexture(QImage("D:/5.png")); //5.jpg

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

	int i = 1000;
	glLoadName(i++);
	glPushMatrix();
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

	shader.enableAttributeArray("uvVertex");
	shader.enableAttributeArray("vertexNormal");
	shader.enableAttributeArray("aPos");
	shader.setUniformValue("glFrontFacing_1", 0);

	// �����
	//glEnable(GL_LINE_SMOOTH);
	//glHint(GL_LINE_SMOOTH, GL_NICEST);

	// �����ƫ��
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	// ģ���������
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPopMatrix();

	shader.setUniformValue("glFrontFacing_1", 1);
	// ���������ߵĿ�� 
	glLineWidth(0.4f);
	// ģ���������
	glDrawElements(GL_LINE_LOOP, indices.size(), GL_UNSIGNED_INT, 0);

	//�����������
	shader.setUniformValue("glFrontFacing_1", 2);
	glLineWidth(0.3);
	glDrawArrays(GL_LINES, PlanStartIndex, 80);

	//X�����
	shader.setUniformValue("glFrontFacing_1", 3);
	glLineWidth(3.0);
	glDrawArrays(GL_LINES, PlanStartIndex + 80, 2);

	//Y�����
	shader.setUniformValue("glFrontFacing_1", 4);
	glDrawArrays(GL_LINES, PlanStartIndex + 82, 2);

	//Z�����
	shader.setUniformValue("glFrontFacing_1", 5);
	glDrawArrays(GL_LINES, PlanStartIndex + 84, 2);

	glLoadName(i++);
	glPushMatrix();
	// set modelview matrix
	matrixViewx.setToIdentity();
	matrixViewx.translate(QVector3D(-14.0, 0.0, -14.0));
	matrixViewx.rotate(rotation);
	matrixViewx.translate(translate.x(), translate.y(), translate.z());
	matrixModelViewProjectionx = matrixProjectionx * matrixViewx;
	matrixNormalx = matrixViewx;
	matrixNormalx.setColumn(3, QVector4D(0, 0, 0, 1));

	glUniformMatrix4fv(uniformMatrixModelView, 1, false, matrixViewx.data());
	glUniformMatrix4fv(uniformMatrixModelViewProjection, 1, false, matrixModelViewProjectionx.data());
	glUniformMatrix4fv(uniformMatrixNormal, 1, false, matrixNormalx.data());

	shader.setUniformValue("glFrontFacing_1", 0);
	// �����ƫ��
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	// ģ���������
	shader.setUniformValue("glFrontFacing_1", 6);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glPopMatrix();
	glDisable(GL_POLYGON_OFFSET_FILL);

	shader.setUniformValue("glFrontFacing_1", 1);
	// ���������ߵĿ�� 
	glLineWidth(0.4f);
	// ģ���������
	//glDrawElements(GL_LINE_LOOP, indices.size(), GL_UNSIGNED_INT, 0);

	shader.disableAttributeArray("aPos");
	shader.disableAttributeArray("vertexNormal");
	shader.disableAttributeArray("uvVertex");
	shader.release();
	indexBuffer->release();
	uvBuffer->release();
	m_texture->release();
}

/*
1. ����������ֵ���Ŀ��һ����������ж�����֣��㶮��
2. �����屻ѡ���������СZֵ�����������������ֵ��֪��һ�������������ڵ�����
3. �����屻ѡ����������Zֵ��
4. ����
5. ���֡���
*/

void OpenglMathMode::SelectObject(GLint x, GLint y)
{
	GLuint selectBuff[32] = { 0 };//����һ������ѡ����������    
	GLint hits, viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport); //���viewport    
	glSelectBuffer(64, selectBuff); //����OpenGL��ʼ��  selectbuffer    
	 //����ѡ��ģʽ    
	glRenderMode(GL_SELECT);
	glInitNames();  //��ʼ������ջ    
	glPushName(-1);  //������ջ�з���һ����ʼ�����֣�����Ϊ��0��    

	glPushMatrix();   //������ǰ��ͶӰ����    
	glMatrixMode(GL_PROJECTION);    //����ͶӰ�׶�׼��ʰȡ    
	glLoadIdentity();   //���뵥λ����    
	float m[16];
	glGetFloatv(GL_PROJECTION_MATRIX, m);  //��ص�ǰ��ͶӰ���� 
	gluPickMatrix(x,           // �趨����ѡ���Ĵ�С������ʰȡ���󣬾�������Ĺ�ʽ  
		viewport[3] - y,    // viewport[3]������Ǵ��ڵĸ߶ȣ���������ת��ΪOpenGL���꣨OPengl��������ϵ��   
		2, 2,              // ѡ���Ĵ�СΪ2��2    
		viewport          // �ӿ���Ϣ�������ӿڵ���ʼλ�úʹ�С    
	);

	gluPerspective(
		100.0, qreal(viewPortWidth) / qreal(viewPortHeight ? viewPortHeight : 1),
		0.01,
		25);
	glGetFloatv(GL_PROJECTION_MATRIX, m);//�鿴��ǰ��ʰȡ����  
	//ͶӰ��������һ������
	glOrtho(-10, 10, -10, 10, -10, 10);     //ʰȡ�������ͶӰ���������Ϳ�����ѡ���Ŵ�Ϊ������һ����    
	glGetFloatv(GL_PROJECTION_MATRIX, m);

	glMatrixMode(GL_MODELVIEW);    // 2: GL_SELECT��ģ����ͼ�任
	glLoadIdentity();

	paintGL();

	glPopMatrix();

	hits = glRenderMode(GL_RENDER); // ��ѡ��ģʽ��������ģʽ,�ú�������ѡ�񵽶���ĸ���


	glMatrixMode(GL_PROJECTION);
	glPopMatrix();  // ����������ͶӰ�任    
	glGetFloatv(GL_PROJECTION_MATRIX, m);//����ԭ��ѡ�����֮ǰ��ͶӰ�任���� 
	hits = glRenderMode(GL_RENDER); // ��ѡ��ģʽ��������ģʽ,�ú�������ѡ�񵽶���ĸ���
	int a = 0;
	//if (hits > 0)
	//	processSelect(selectBuff);  //  ѡ��������
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

	//����������������
	setWorldGrid();

	// �������黺������
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

	// ����������
	uvBuffer->create();
	uvBuffer->bind();
	uvBuffer->allocate(&uvData[0], sizeof(float) * uvData.size());
	uvBuffer->setUsagePattern(QGLBuffer::StaticDraw);
	shader.setAttributeBuffer("uvVertex", GL_FLOAT, 2 * sizeof(GL_FLOAT), 2, 2 * sizeof(GL_FLOAT));
	uvBuffer->release();

	// ����������������
	indexBuffer->create();
	indexBuffer->bind();
	indexBuffer->allocate(&indices[0], sizeof(int) * indices.size());
	indexBuffer->setUsagePattern(QGLBuffer::StaticDraw);
	indexBuffer->release();
}

/*
	//ͶӰ����
	QMatrix4x4 projection;
	projection.perspective(60,  //������Ұ�ĽǶ� ������ͷ������������£���Ұ���Ϊ60��
	(float)width()/height(), //���ڱ���
	0.1f,100);//���������ı�ʾ����������������Զ��Χ�ڵ�����Ҫ����ʾ�����������Χ�����彫�޷�����ʾ��

LookAt���������������ռ�����ת��Ϊ�۲�ռ����꣬ʵ�����������Ϊԭ�����¶������ά�ռ䡣
glm::mat4 LookAt(glm::vec3 pos, glm::vec3 target, glm::vec3 up)
���������������������λ��pos��Ŀ��λ��target�Լ����������up�����������ǣ�glm::vec3 zaxis = normalize(pos-target)ָ��۲�ռ���Z�᷽��glm::vec3 xaxis = normalize(glm::cross(up, zaxis))ָ��۲�ռ���X�᷽��glm::vec3 = glm::cross(zaxis, xaxis)ָ��۲�ռ���Y�᷽��
���ϵ����������ߵ�������z�ᣩȷ��һ��ƽ�棬y��һ�������ƽ���ϡ�ͨ�����ƽ��ȷ�����ҷ���x�ᣩ������z�ᡢx�ᣬy���ȷ���ˡ�
�߳�ȥ��������������ƽ���ڵײ�����ͷ��������б����ƽ��������ת������ʸ������ͷ���ķ���
�ǡ���������ģ�������ʹ������������������������񳡾�Χ��������ת��
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
	SelectObject(mousePressPosition.x(), mousePressPosition.y());
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
gl_Position ����û������in��out����uniform������������ֱ��ʹ�ã����ں���ĳ�����Ҳδ�����á�ԭ������Ĭ���ǹ�һ���Ĳü��ռ����꣬xyz����ά�ȵķ�ΧΪ - 1��1�������ڶ�����ɫ����ʹ�ã���������Ҳ�������gl_Position��ֵ��Χ����float��ȡֵ��Χ(32λ)��ֻ����ֻ��[-1, 1]�����ƬԪ�����ơ�����vec4���͵ģ�����������Ϊdvec4�����͡�
gl_Position����ͨ���ӽǻ���ת��Ϊ��׼���豸�ռ��еĵѿ������꣺
vec3 ndc = gl_Position.xyz / gl_Position.w;
*/