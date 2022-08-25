#include "openglmathmode.h"
#include "ui_openglmathmode.h"
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

	// �����
	//glEnable(GL_LINE_SMOOTH);
	//glHint(GL_LINE_SMOOTH, GL_NICEST);

	// �����ƫ��
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	// ģ���������
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glDisable(GL_POLYGON_OFFSET_FILL);

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
	float v_l = Vmax - Vmin;

	// ������ֳ����ɶν��м���
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
			// ���㶥������
			vertices[step * 6 * i + j * 6] = Fx;
			vertices[step * 6 * i + j * 6 + 1] = Fy;
			vertices[step * 6 * i + j * 6 + 2] = Fz;
			vertices[step * 6 * i + j * 6 + 3] = 0.0f;
			vertices[step * 6 * i + j * 6 + 4] = 0.0f;
			vertices[step * 6 * i + j * 6 + 5] = 0.0f;

			// ������������
			if (i < step - 1) {
				// D C
				// A B 
				// ������1
				indices[step * 6 * i + j * 6] = j + step * i;        // A
				indices[step * 6 * i + j * 6 + 1] = (j + 1) % step + step * i; // B
				indices[step * 6 * i + j * 6 + 2] = (j + 1) % step + step * i + step; // C

				// ������2
				indices[step * 6 * i + j * 6 + 3] = j + step * i; // A
				indices[step * 6 * i + j * 6 + 4] = j + step * i + step; // D
				indices[step * 6 * i + j * 6 + 5] = (j + 1) % step + step * i + step; // C

				// ��������
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
				���ֶ���ó�������
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

			//��֤��ȷ�������    ���߼����д��󣬷�����
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

	// X�����
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);

	// X����� ������
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(1.0f);

	// X���յ�
	vertices.push_back(10.0f);
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);

	// X���յ� ������
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(1.0f);

	// Y��
	// ���
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);

	// Y����� ������
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(1.0f);

	vertices.push_back(0.0f);
	vertices.push_back(10.0f);
	vertices.push_back(0.0f);

	// Y���յ� ������
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(1.0f);

	// Z�����
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);

	// Z����� ������
	vertices.push_back(1.0f);
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);

	// Z���յ�
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(10.0f);

	// Z����� ������
	vertices.push_back(1.0f);
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
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