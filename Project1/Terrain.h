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
//地形类 

class CTerrain

{

public:

	//构造函数 

	CTerrain() {
		// 设置地形大小 
		setSize(100, 1);

		// 为地形高程分配内存,并初始化 

		m_pHeightMap = new GLbyte[m_nWidth * m_nWidth];

		for (int i = 0; i < m_nWidth; i++) {
			m_pHeightMap[i] = 0;
		}
	}

	~CTerrain() {
		// 删除内存和指针 
		if (m_pHeightMap)
		{
			delete[] m_pHeightMap;
			m_pHeightMap = 0;
		}
		// 删除纹理对象及其占用内存  
		stbi_image_free(textureData);
		glDeleteTextures(1, &m_texture);
	}

	//设置地形的大小 

	void setSize(unsigned  int width, unsigned  int cell){
		m_nWidth = width;
		m_nCellWidth = cell;
	}

	// 初始化地形 

	bool initTerrain() {
		// 载入高度文件 
		loadRawFile("./terrain/heightmap.png");
		// 载入纹理 
		//loadTexture("./terrain/rock.jpg");
		return true;
	}

	// 载入'.raw'高度图 
	bool loadRawFile(const char* fileName) {
		FILE *pFile = NULL;
		// 打开文件 
		fopen_s(&pFile,fileName, "rb");
		// 错误检查 
		if (pFile == NULL)
		{
			// 输出错误信息，并返回false 
			//MessageBox(NULL, "打开高度图文件失败！", "错误", MB_OK);
			exit(0);
		}
		// 读取高度图文件 
		fread(m_pHeightMap, 1, m_nWidth*m_nWidth, pFile);
		// 获取错误代码 
		int result = ferror(pFile);
		// 检查错误代码 
		if (result)
		{
			//MessageBox(NULL, "无法获取高度数据！", "错误", MB_OK);
			exit(0);
		}
		// 关闭文件，成功返回 
		fclose(pFile);
		return true;
	}

	// 装载地形纹理 

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

	// 获得点(x,y)的高度信息 

	int getHeight(int x, int y) {
		if (!m_pHeightMap)

			return 0;

		int xx = x % m_nWidth;

		int yy = y % m_nWidth;

		// 返回当前高度 

		return m_pHeightMap[(xx + yy * m_nWidth)];
	}

	// 获得地面高度 

	float getAveHeight(float x, float z) {
		float CameraX, CameraZ;

		CameraX = x / m_nCellWidth;

		CameraZ = z / m_nCellWidth;

		// 计算高程坐标(Col0, Row0)，(Col1, Row1) 

		int col0 = int(CameraX);

		int row0 = int(CameraZ);

		unsigned int col1 = col0 + 1;

		unsigned int row1 = row0 + 1;

		// 确保单元坐标不超过高程以外 

		if (col1 > m_nWidth) col1 = 0;

		if (row1 > m_nWidth) row1 = 0;

		// 获取单元的四个角的高度 

		float h00 = (float)(m_pHeightMap[col0*m_nCellWidth + row0*m_nCellWidth*m_nWidth]);

		float h01 = (float)(m_pHeightMap[col1*m_nCellWidth + row0*m_nCellWidth*m_nWidth]);

		float h11 = (float)(m_pHeightMap[col1*m_nCellWidth + row1*m_nCellWidth*m_nWidth]);

		float h10 = (float)(m_pHeightMap[col0*m_nCellWidth + row1*m_nCellWidth*m_nWidth]);

		// 计算机摄像机相对于单元格的位置 

		float tx = CameraX - int(CameraX);

		float ty = CameraZ - int(CameraZ);

		// 进行双线性插值得到地面高度 

		float txty = tx * ty;

		float final_height = h00 * (1.0f - ty - tx + txty)

			+ h01 * (tx - txty)

			+ h11 * txty

			+ h10 * (ty - txty);

		return final_height;
	}

	// 设置纹理坐标 

	void setTexCoord(float x, float z) {
		m_vertarray.push_back(x);
		m_vertarray.push_back(z);
	}

	// 设置顶点坐标
	void setVertCoord(float x, float y, float z) {
		m_vertarray.push_back(x);
		m_vertarray.push_back(y);
		m_vertarray.push_back(z);
	}

	// 渲染地形 

	void render() {
		int X = 0, Y = 0;
		float x, y, z;
		bool bSwitchSides = false;
		// 检查高度图是否有效 
		if (!m_pHeightMap)
			return;
		// 绑定纹理 
		//glBindTexture(GL_TEXTURE_2D, m_texture);
		//glDisable();                          // 关闭光照 
		//glBegin(GL_TRIANGLE_STRIP);
		// 从行(X)开始循环 
		for (X = 0; X <= m_nWidth; X += m_nCellWidth)
		{
			// 检查该列是否需要从相反顺序绘制 
			if (bSwitchSides)
			{
				//绘制地形的一列 
				for (Y = m_nWidth; Y >= 0; Y -= m_nCellWidth)
				{
					//顶点(X, Y, Z)    
					x = X;
					y = getHeight(X, Y);
					z = Y;
					// 指定纹理坐标,并发送顶点 
					setTexCoord((float)x, (float)z);
					setVertCoord(x, y, z);
					// 顶点(X + m_nCellWidth, Y, Z)     
					x = X + m_nCellWidth;
					y = getHeight(X + m_nCellWidth, Y);
					z = Y;
					// 指定纹理坐标,并发送顶点 
					setTexCoord((float)x, (float)z);
					setVertCoord(x, y, z);
				}
			}
			else
			{
				// 绘制地形的一列 
				for (Y = 0; Y <= m_nWidth; Y += m_nCellWidth)
				{
					// 顶点(X + m_nCellWidth, Y, Z)
					x = X + m_nCellWidth;
					y = getHeight(X + m_nCellWidth, Y);
					z = Y;
					// 指定纹理坐标,并发送顶点 
					setTexCoord((float)x, (float)z);
					setVertCoord(x, y, z);
					// 顶点 (X, Y, Z) */     
					x = X;
					y = getHeight(X, Y);
					z = Y;
					//指定纹理坐标,并发送顶点 
					setTexCoord((float)x, (float)z);
					setVertCoord(x, y, z);
				}
			}
			// 变换开关 
			bSwitchSides = !bSwitchSides;
		}
		//绘制结束
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

	unsigned  int   m_nWidth;          //地形网格数 

	unsigned  int   m_nCellWidth;      // 每一格宽度 

	GLbyte*         m_pHeightMap;      //存放高度信息 

	unsigned  int   m_texture;         //载入纹理 

	unsigned char   *textureData;

	std::vector<float> m_vertarray;
};

#endif