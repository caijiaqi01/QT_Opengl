#include "calculate.h"
#include "ui_openglmathmode.h"
#include<sstream>
#include <cmath>

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
	for (i = 0; i + 1 < step; i++) {
		for (j = 0; j + 1 < step; j++) {
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
		vertices.push_back(-11 * step); // y
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
