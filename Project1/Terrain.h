#pragma once
#ifndef TERRAIN_H
#define TERRAIN_H
#define _CRT_SECURE_NO_DEPRECATE
#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include <iostream>
#include <vector>
//������ 

class CTerrain

{

public:

	//���캯�� 

	CTerrain() {
		// ���õ��δ�С 
		setSize(100, 1);

		// Ϊ���θ̷߳����ڴ�,����ʼ�� 

		m_pHeightMap = new GLbyte[m_nWidth * m_nWidth];

		for (int i = 0; i < m_nWidth; i++) {
			m_pHeightMap[i] = 0;
		}
	}

	~CTerrain() {
		// ɾ���ڴ��ָ�� 
		if (m_pHeightMap)
		{
			delete[] m_pHeightMap;
			m_pHeightMap = 0;
		}
		// ɾ�����������ռ���ڴ�  
		stbi_image_free(textureData);
		glDeleteTextures(1, &m_texture);
	}

	//���õ��εĴ�С 

	void setSize(unsigned  int width, unsigned  int cell){
		m_nWidth = width;
		m_nCellWidth = cell;
	}

	// ��ʼ������ 

	bool initTerrain() {
		// ����߶��ļ� 
		loadRawFile("./terrain/heightmap.png");
		// �������� 
		//loadTexture("./terrain/rock.jpg");
		return true;
	}

	// ����'.raw'�߶�ͼ 
	bool loadRawFile(const char* fileName) {
		FILE *pFile = NULL;
		// ���ļ� 
		fopen_s(&pFile,fileName, "rb");
		// ������ 
		if (pFile == NULL)
		{
			// ���������Ϣ��������false 
			//MessageBox(NULL, "�򿪸߶�ͼ�ļ�ʧ�ܣ�", "����", MB_OK);
			exit(0);
		}
		// ��ȡ�߶�ͼ�ļ� 
		fread(m_pHeightMap, 1, m_nWidth*m_nWidth, pFile);
		// ��ȡ������� 
		int result = ferror(pFile);
		// ��������� 
		if (result)
		{
			//MessageBox(NULL, "�޷���ȡ�߶����ݣ�", "����", MB_OK);
			exit(0);
		}
		// �ر��ļ����ɹ����� 
		fclose(pFile);
		return true;
	}

	// װ�ص������� 

	bool loadTexture(const char * fileName) {
		glGenTextures(1, &m_texture);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// load image, create texture and generate mipmaps
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
		unsigned char *data = stbi_load(fileName, &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}

		return true;
	}

	// ��õ�(x,y)�ĸ߶���Ϣ 

	int getHeight(int x, int y) {
		if (!m_pHeightMap)

			return 0;

		int xx = x % m_nWidth;

		int yy = y % m_nWidth;

		// ���ص�ǰ�߶� 

		return m_pHeightMap[(xx + yy * m_nWidth)];
	}

	// ��õ���߶� 

	float getAveHeight(float x, float z) {
		float CameraX, CameraZ;

		CameraX = x / m_nCellWidth;

		CameraZ = z / m_nCellWidth;

		// ����߳�����(Col0, Row0)��(Col1, Row1) 

		int col0 = int(CameraX);

		int row0 = int(CameraZ);

		unsigned int col1 = col0 + 1;

		unsigned int row1 = row0 + 1;

		// ȷ����Ԫ���겻�����߳����� 

		if (col1 > m_nWidth) col1 = 0;

		if (row1 > m_nWidth) row1 = 0;

		// ��ȡ��Ԫ���ĸ��ǵĸ߶� 

		float h00 = (float)(m_pHeightMap[col0*m_nCellWidth + row0*m_nCellWidth*m_nWidth]);

		float h01 = (float)(m_pHeightMap[col1*m_nCellWidth + row0*m_nCellWidth*m_nWidth]);

		float h11 = (float)(m_pHeightMap[col1*m_nCellWidth + row1*m_nCellWidth*m_nWidth]);

		float h10 = (float)(m_pHeightMap[col0*m_nCellWidth + row1*m_nCellWidth*m_nWidth]);

		// ��������������ڵ�Ԫ���λ�� 

		float tx = CameraX - int(CameraX);

		float ty = CameraZ - int(CameraZ);

		// ����˫���Բ�ֵ�õ�����߶� 

		float txty = tx * ty;

		float final_height = h00 * (1.0f - ty - tx + txty)

			+ h01 * (tx - txty)

			+ h11 * txty

			+ h10 * (ty - txty);

		return final_height;
	}

	// ������������ 

	void setTexCoord(float x, float z) {
		m_vertarray.push_back(x);
		m_vertarray.push_back(z);
	}

	// ���ö�������
	void setVertCoord(float x, float y, float z) {
		m_vertarray.push_back(x);
		m_vertarray.push_back(y);
		m_vertarray.push_back(z);
	}

	// ��Ⱦ���� 

	void render() {
		int X = 0, Y = 0;
		float x, y, z;
		bool bSwitchSides = false;
		// ���߶�ͼ�Ƿ���Ч 
		if (!m_pHeightMap)
			return;
		// ������ 
		//glBindTexture(GL_TEXTURE_2D, m_texture);
		//glDisable();                          // �رչ��� 
		//glBegin(GL_TRIANGLE_STRIP);
		// ����(X)��ʼѭ�� 
		for (X = 0; X <= m_nWidth; X += m_nCellWidth)
		{
			// �������Ƿ���Ҫ���෴˳����� 
			if (bSwitchSides)
			{
				//���Ƶ��ε�һ�� 
				for (Y = m_nWidth; Y >= 0; Y -= m_nCellWidth)
				{
					//����(X, Y, Z)    
					x = X;
					y = getHeight(X, Y);
					z = Y;
					// ָ����������,�����Ͷ��� 
					setTexCoord((float)x, (float)z);
					setVertCoord(x, y, z);
					// ����(X + m_nCellWidth, Y, Z)     
					x = X + m_nCellWidth;
					y = getHeight(X + m_nCellWidth, Y);
					z = Y;
					// ָ����������,�����Ͷ��� 
					setTexCoord((float)x, (float)z);
					setVertCoord(x, y, z);
				}
			}
			else
			{
				// ���Ƶ��ε�һ�� 
				for (Y = 0; Y <= m_nWidth; Y += m_nCellWidth)
				{
					// ����(X + m_nCellWidth, Y, Z)
					x = X + m_nCellWidth;
					y = getHeight(X + m_nCellWidth, Y);
					z = Y;
					// ָ����������,�����Ͷ��� 
					setTexCoord((float)x, (float)z);
					setVertCoord(x, y, z);
					// ���� (X, Y, Z) */     
					x = X;
					y = getHeight(X, Y);
					z = Y;
					//ָ����������,�����Ͷ��� 
					setTexCoord((float)x, (float)z);
					setVertCoord(x, y, z);
				}
			}
			// �任���� 
			bSwitchSides = !bSwitchSides;
		}
		//���ƽ���
		//glEnd();
	}

	std::vector<float> getVertex() {
		return m_vertarray;
	}

	unsigned int getTexture() {
		return m_texture;
	}

	unsigned  int getWidth() {
		return m_nWidth;
	}
private:

	unsigned  int   m_nWidth;          //���������� 

	unsigned  int   m_nCellWidth;      // ÿһ���� 

	GLbyte*         m_pHeightMap;      //��Ÿ߶���Ϣ 

	unsigned  int   m_texture;         //�������� 

	unsigned char   *textureData;

	std::vector<float> m_vertarray;
};

#endif