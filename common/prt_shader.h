#ifndef PRT_ALGORITHM_H
#define PRT_ALGORITHM_H

#include "Shader.h"
#include "Model.h"
#include "Camera.h"
#include "texture.h"
#include "lamptexture.h"
#include "bvh_tree.h"

#include <iostream>
#include <cmath>
#include <string>
#include <ctime>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Prt{
private:
    int lmax = 2;
    const static int samps = 25;

    std::vector<float> phi, theta;
    std::vector<float> p_coeff;
    std::vector<float> k_coeff;
    std::vector<float> y_coeff[samps];
    std::vector<float> cos_coeff;
    std::vector<float> sin_coeff;
    std::vector<glm::vec3> light_coeff, transfer_coeff;

	GLuint screenWidth = 800, screenHeight = 600;
	Shader modelshader;
	Shader boxshader;
	Model ourModel;
    Model ourFloor;
    BvhTree bvhTree;
    Camera camera;
    
    LampTexture* lamptexture[6];

    GLfloat envmap[12] = {
        -1.f, -1.f, 0.f,
        -1.f, 1.f, 0.f,
        1.f, -1.f, 0.f,
        1.f, 1.f, 0.f
    };

    GLuint indices[6] = { 
        0, 2, 1, 
        1, 2, 3  
    };

	GLfloat skyboxVertices[108] = {
        // Positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
  
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
  
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
   
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
  
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
  
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    

	GLuint skyboxVAO, cubemapTexture, skyboxVBO;
    GLuint vertexArrayID, vertexBufferPositionID, vertexBufferIndicesID;
	std::vector<const GLchar*> faces;

    void convert_xyz_to_cube_uv(glm::vec3 dir, int *index, float *u, float *v)
    {
      float x = dir.x;
      float y = dir.y;
      float z = dir.z;

      float absX = fabs(x);
      float absY = fabs(y);
      float absZ = fabs(z);
      
      int isXPositive = x > 0 ? 1 : 0;
      int isYPositive = y > 0 ? 1 : 0;
      int isZPositive = z > 0 ? 1 : 0;
      
      float maxAxis, uc, vc;
      
      // POSITIVE X
      if (isXPositive && absX >= absY && absX >= absZ) {
        // u (0 to 1) goes from +z to -z
        // v (0 to 1) goes from -y to +y
        maxAxis = absX;
        uc = -z;
        vc = y;
        *index = 0;
      }
      // NEGATIVE X
      if (!isXPositive && absX >= absY && absX >= absZ) {
        // u (0 to 1) goes from -z to +z
        // v (0 to 1) goes from -y to +y
        maxAxis = absX;
        uc = z;
        vc = y;
        *index = 1;
      }
      // POSITIVE Y
      if (isYPositive && absY >= absX && absY >= absZ) {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from +z to -z
        maxAxis = absY;
        uc = x;
        vc = -z;
        *index = 2;
      }
      // NEGATIVE Y
      if (!isYPositive && absY >= absX && absY >= absZ) {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from -z to +z
        maxAxis = absY;
        uc = x;
        vc = z;
        *index = 3;
      }
      // POSITIVE Z
      if (isZPositive && absZ >= absX && absZ >= absY) {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from -y to +y
        maxAxis = absZ;
        uc = x;
        vc = y;
        *index = 4;
      }
      // NEGATIVE Z
      if (!isZPositive && absZ >= absX && absZ >= absY) {
        // u (0 to 1) goes from +x to -x
        // v (0 to 1) goes from -y to +y
        maxAxis = absZ;
        uc = -x;
        vc = y;
        *index = 5;
      }

      // Convert range from -1 to 1 to 0 to 1
      *u = 0.5f * (uc / maxAxis + 1.0f);
      *v = 0.5f * (vc / maxAxis + 1.0f);
    }

    int p(int l,int m) 
    {
        return l * l + l + m;
    }

    void generate_sample_angles() 
    {
        int sqrtSamps = (int)sqrtf(1.f * samps);
        for (int i = 0; i < sqrtSamps; ++i)
            for (int j = 0; j < sqrtSamps; ++j) 
            {
                float x = (1.f * rand() / RAND_MAX + 1.f * i) / float(sqrtSamps);
                float y = (1.f * rand() / RAND_MAX + 1.f * j) / float(sqrtSamps);
                theta.push_back(2.f * acos(sqrt(1.f - x)));
                phi.push_back(y * 2.f * M_PI);
            }
    }

    void calc_sh_coeff(float theta_, float phi_, std::vector<float>& y_coeff) 
    {
        for (int j = 0; j < (lmax + 1) * (lmax + 1); ++j) {
            y_coeff.push_back(0.f);
        }
        float x = cos(theta_);

        // step 1
        p_coeff[p(0, 0)] = 1.f;
        p_coeff[p(1, 0)] = x;
        for (int l = 2; l <= lmax; ++l)
            p_coeff[p(l, 0)] =
                ((2.f * l - 1) * x * p_coeff[p(l - 1, 0)] -
                (1.f * l - 1) * p_coeff[p(l - 2, 0)]) / float(l);
        // step 2
        float neg = -1.f;
        float dfact = 1.f;
        float xroot = sqrtf(std::max(0.f, 1.f - x * x));
        float xpow = xroot;
        for (int l = 1; l <= lmax; ++l) {
            p_coeff[p(l, l)] = neg * dfact * xpow;
            neg *= -1.f;
            dfact *= 2.f * l + 1.f;
            xpow *= xroot;
        }
        // step 3
        for (int l = 2; l <= lmax; ++l)
            p_coeff[p(l, l - 1)] =
                x * (2.f * l - 1.f) * p_coeff[p(l - 1, l - 1)];
        // step 4
        for (int l = 3; l <= lmax; ++l)
            for (int m = 1; m <= l - 2; ++m)
                p_coeff[p(l, m)] =
                    ((2.f * (l - 1.f) + 1.f) * x * p_coeff[p(l - 1, m)] -
                    (1.f * l - 1.f + m) * p_coeff[p(l - 2, m)]) / (l - m);
        // step 5
        for (int l = 0; l <= lmax; ++l)
            for (int m = -l; m <= l; ++m) {
                float k = (2.f * l + 1.f) / 4.f / M_PI;
                for (int i = l - abs(m) + 1; i <= l + abs(m); ++i)
                    k /= float(i);
                k = sqrtf(k);
                k_coeff[p(l, m)] = k;
            }
        // step 6
        sin_coeff[0] = 0;
        cos_coeff[0] = 1;
        sin_coeff[1] = sin(phi_);
        cos_coeff[1] = cos(phi_);
        for (int l = 2; l <= lmax; ++l) {
            sin_coeff[l] = sin_coeff[l - 1] * cos_coeff[1] + cos_coeff[l - 1] * sin_coeff[1];
            cos_coeff[l] = cos_coeff[l - 1] * cos_coeff[1] - sin_coeff[l - 1] * sin_coeff[1];
        }
        // step 7
        for (int l = 0; l <= lmax; ++l) {
            for (int m = -l; m < 0; ++m) {
                y_coeff[p(l, m)] = sqrtf(2.f) * k_coeff[p(l, m)] * p_coeff[p(l, -m)] * sin_coeff[-m];
            }
            y_coeff[p(l, 0)] = k_coeff[p(l, 0)] * p_coeff[p(l, 0)];
            for (int m = 1; m <= l; ++m) {
                y_coeff[p(l, m)] = sqrtf(2.f) * k_coeff[p(l, m)] * p_coeff[p(l, m)] * cos_coeff[m];
            }
        }
    }

    void generate_sh()
    {
        for (int i = 0; i < (lmax + 1) * (lmax + 1); ++i) {
            p_coeff.push_back(0.f);
            k_coeff.push_back(0.f);
            cos_coeff.push_back(0.f);
            sin_coeff.push_back(0.f);
            light_coeff.push_back(glm::vec3());
            transfer_coeff.push_back(glm::vec3());
        }

        for (int i = 0; i < samps; ++i) {
            calc_sh_coeff(theta[i],phi[i],y_coeff[i]);
        }
    }

    void rotate_z(const std::vector<glm::vec3>& vin, std::vector<glm::vec3>& vout, float angle) {
        sin_coeff[0] = 0;
        cos_coeff[0] = 1;
        sin_coeff[1] = sin(angle);
        cos_coeff[1] = cos(angle);
        for (int l = 2; l <= lmax; ++l) {
            sin_coeff[l] = sin_coeff[l - 1] * cos_coeff[1] + cos_coeff[l - 1] * sin_coeff[1];
            cos_coeff[l] = cos_coeff[l - 1] * cos_coeff[1] - sin_coeff[l - 1] * sin_coeff[1];
        }
        for (int i = 0; i < (lmax + 1) * (lmax + 1); ++i) {
            vout.push_back(glm::vec3(0, 0, 0));
        }
        vout[0] = vin[0];
        for (int l = 1; l <= lmax; ++l) {
            for (int m = -l; m < 0; ++m)
                vout[p(l, m)] = cos_coeff[-m] * vin[p(l, m)] - sin_coeff[-m] * vin[p(l, -m)];
            vout[p(l, 0)] = vin[p(l, 0)];
            for (int m = 1; m <= l; ++m)
                vout[p(l, m)] = cos_coeff[m] * vin[p(l, m)] + sin_coeff[m] * vin[p(l, -m)];
        }
    }

    void rotate_x_minus(const std::vector<glm::vec3>& vin, std::vector<glm::vec3>& vout) {
        rotate_x_plus(vin, vout);
        for (int l = 1; l <= lmax; ++l) {
            float s = (l & 0x1) ? -1.f : 1.f;
            vout[p(l, 0)] *= s;
            for (int m = 1; m <= l; ++m) {
                s = -s;
                vout[p(l, m)] *= s;
                vout[p(l, -m)] *= -s;
            }
        }
    }

    void rotate_x_plus(const std::vector<glm::vec3>& vin, std::vector<glm::vec3>& vout) {
        if (lmax > 9) {
            fprintf(stderr, "Error, do not support lmax > 9.\n");
        }
        for (int i = 0; i < (lmax + 1) * (lmax + 1); ++i) {
            vout.push_back(glm::vec3(0,0,0));
        }

    #define O(l, m)  vin[p(l, m)]

        int index = 0;
        // first band is a no-op
        vout[index++] = vin[0];

        if (lmax < 1) return;
        vout[index++] = (O(1, 0));
        vout[index++] = (-1.f*O(1, -1));
        vout[index++] = (O(1, 1));

        if (lmax < 2) return;
        vout[index++] = (O(2, 1));
        vout[index++] = (-1.f*O(2, -1));
        vout[index++] = (-0.5f*O(2, 0) - 0.8660254037844386f*O(2, 2));
        vout[index++] = (-1.f*O(2, -2));
        vout[index++] = (-0.8660254037844386f*O(2, 0) + 0.5f*O(2, 2));

        // Remainder of SH $x+$ rotation definition
        if (lmax < 3) return;
        vout[index++] = (-0.7905694150420949f*O(3, 0) + 0.6123724356957945f*O(3, 2));
        vout[index++] = (-1.f*O(3, -2));
        vout[index++] = (-0.6123724356957945f*O(3, 0) - 0.7905694150420949f*O(3, 2));
        vout[index++] = (0.7905694150420949f*O(3, -3) + 0.6123724356957945f*O(3, -1));
        vout[index++] = (-0.25f*O(3, 1) - 0.9682458365518543f*O(3, 3));
        vout[index++] = (-0.6123724356957945f*O(3, -3) + 0.7905694150420949f*O(3, -1));
        vout[index++] = (-0.9682458365518543f*O(3, 1) + 0.25f*O(3, 3));

        if (lmax < 4) return;
        vout[index++] = (-0.9354143466934853f*O(4, 1) + 0.35355339059327373f*O(4, 3));
        vout[index++] = (-0.75f*O(4, -3) + 0.6614378277661477f*O(4, -1));
        vout[index++] = (-0.35355339059327373f*O(4, 1) - 0.9354143466934853f*O(4, 3));
        vout[index++] = (0.6614378277661477f*O(4, -3) + 0.75f*O(4, -1));
        vout[index++] = (0.375f*O(4, 0) + 0.5590169943749475f*O(4, 2) + 0.739509972887452f*O(4, 4));
        vout[index++] = (0.9354143466934853f*O(4, -4) + 0.35355339059327373f*O(4, -2));
        vout[index++] = (0.5590169943749475f*O(4, 0) + 0.5f*O(4, 2) - 0.6614378277661477f*O(4, 4));
        vout[index++] = (-0.35355339059327373f*O(4, -4) + 0.9354143466934853f*O(4, -2));
        vout[index++] = (0.739509972887452f*O(4, 0) - 0.6614378277661477f*O(4, 2) + 0.125f*O(4, 4));

        if (lmax < 5) return;
        vout[index++] = (0.701560760020114f*O(5, 0) - 0.6846531968814576f*O(5, 2) +
            0.19764235376052372f*O(5, 4));
        vout[index++] = (-0.5f*O(5, -4) + 0.8660254037844386f*O(5, -2));
        vout[index++] = (0.5229125165837972f*O(5, 0) + 0.30618621784789724f*O(5, 2) -
            0.795495128834866f*O(5, 4));
        vout[index++] = (0.8660254037844386f*O(5, -4) + 0.5f*O(5, -2));
        vout[index++] = (0.4841229182759271f*O(5, 0) + 0.6614378277661477f*O(5, 2) +
            0.57282196186948f*O(5, 4));
        vout[index++] = (-0.701560760020114f*O(5, -5) - 0.5229125165837972f*O(5, -3) -
            0.4841229182759271f*O(5, -1));
        vout[index++] = (0.125f*O(5, 1) + 0.4050462936504913f*O(5, 3) + 0.9057110466368399f*O(5, 5));
        vout[index++] = (0.6846531968814576f*O(5, -5) - 0.30618621784789724f*O(5, -3) -
            0.6614378277661477f*O(5, -1));
        vout[index++] = (0.4050462936504913f*O(5, 1) + 0.8125f*O(5, 3) - 0.4192627457812106f*O(5, 5));
        vout[index++] = (-0.19764235376052372f*O(5, -5) + 0.795495128834866f*O(5, -3) -
            0.57282196186948f*O(5, -1));
        vout[index++] = (0.9057110466368399f*O(5, 1) - 0.4192627457812106f*O(5, 3) + 0.0625f*O(5, 5));

        if (lmax < 6) return;
        vout[index++] = (0.879452954966893f*O(6, 1) - 0.46351240544347894f*O(6, 3) +
            0.10825317547305482f*O(6, 5));
        vout[index++] = (-0.3125f*O(6, -5) + 0.8028270361665706f*O(6, -3) - 0.5077524002897476f*O(6, -1));
        vout[index++] = (0.4330127018922193f*O(6, 1) + 0.6846531968814576f*O(6, 3) -
            0.5863019699779287f*O(6, 5));
        vout[index++] = (0.8028270361665706f*O(6, -5) - 0.0625f*O(6, -3) - 0.5929270612815711f*O(6, -1));
        vout[index++] = (0.19764235376052372f*O(6, 1) + 0.5625f*O(6, 3) + 0.8028270361665706f*O(6, 5));
        vout[index++] = (-0.5077524002897476f*O(6, -5) - 0.5929270612815711f*O(6, -3) -
            0.625f*O(6, -1));
        vout[index++] = (-0.3125f*O(6, 0) - 0.45285552331841994f*O(6, 2) - 0.49607837082461076f*O(6, 4) -
            0.6716932893813962f*O(6, 6));
        vout[index++] = (-0.879452954966893f*O(6, -6) - 0.4330127018922193f*O(6, -4) -
            0.19764235376052372f*O(6, -2));
        vout[index++] = (-0.45285552331841994f*O(6, 0) - 0.53125f*O(6, 2) - 0.1711632992203644f*O(6, 4) +
            0.6952686081652184f*O(6, 6));
        vout[index++] = (0.46351240544347894f*O(6, -6) - 0.6846531968814576f*O(6, -4) -
            0.5625f*O(6, -2));
        vout[index++] = (-0.49607837082461076f*O(6, 0) - 0.1711632992203644f*O(6, 2) +
            0.8125f*O(6, 4) - 0.2538762001448738f*O(6, 6));
        vout[index++] = (-0.10825317547305482f*O(6, -6) + 0.5863019699779287f*O(6, -4) -
            0.8028270361665706f*O(6, -2));
        vout[index++] = (-0.6716932893813962f*O(6, 0) + 0.6952686081652184f*O(6, 2) -
            0.2538762001448738f*O(6, 4) + 0.03125f*O(6, 6));

        if (lmax < 7) return;
        vout[index++] = (-0.6472598492877494f*O(7, 0) + 0.6991205412874092f*O(7, 2) -
            0.2981060004427955f*O(7, 4) + 0.05846339666834283f*O(7, 6));
        vout[index++] = (-0.1875f*O(7, -6) + 0.6373774391990981f*O(7, -4) - 0.7473912964438374f*O(7, -2));
        vout[index++] = (-0.47495887979908324f*O(7, 0) - 0.07328774624724109f*O(7, 2) +
            0.78125f*O(7, 4) - 0.3983608994994363f*O(7, 6));
        vout[index++] = (0.6373774391990981f*O(7, -6) - 0.5f*O(7, -4) - 0.5863019699779287f*O(7, -2));
        vout[index++] = (-0.42961647140211f*O(7, 0) - 0.41984465132951254f*O(7, 2) +
            0.10364452469860624f*O(7, 4) + 0.7927281808728639f*O(7, 6));
        vout[index++] = (-0.7473912964438374f*O(7, -6) - 0.5863019699779287f*O(7, -4) -
            0.3125f*O(7, -2));
        vout[index++] = (-0.41339864235384227f*O(7, 0) - 0.5740991584648073f*O(7, 2) -
            0.5385527481129402f*O(7, 4) - 0.4576818286211503f*O(7, 6));
        vout[index++] = (0.6472598492877494f*O(7, -7) + 0.47495887979908324f*O(7, -5) +
            0.42961647140211f*O(7, -3) + 0.41339864235384227f*O(7, -1));
        vout[index++] = (-0.078125f*O(7, 1) - 0.24356964481437335f*O(7, 3) - 0.4487939567607835f*O(7, 5) -
            0.8562442974262661f*O(7, 7));
        vout[index++] = (-0.6991205412874092f*O(7, -7) + 0.07328774624724109f*O(7, -5) +
            0.41984465132951254f*O(7, -3) + 0.5740991584648073f*O(7, -1));
        vout[index++] = (-0.24356964481437335f*O(7, 1) - 0.609375f*O(7, 3) - 0.5700448858423344f*O(7, 5) +
            0.4943528756111367f*O(7, 7));
        vout[index++] = (0.2981060004427955f*O(7, -7) - 0.78125f*O(7, -5) - 0.10364452469860624f*O(7, -3) +
            0.5385527481129402f*O(7, -1));
        vout[index++] = (-0.4487939567607835f*O(7, 1) - 0.5700448858423344f*O(7, 3) + 0.671875f*O(7, 5) -
            0.14905300022139775f*O(7, 7));
        vout[index++] = (-0.05846339666834283f*O(7, -7) + 0.3983608994994363f*O(7, -5) -
            0.7927281808728639f*O(7, -3) + 0.4576818286211503f*O(7, -1));
        vout[index++] = (-0.8562442974262661f*O(7, 1) + 0.4943528756111367f*O(7, 3) -
            0.14905300022139775f*O(7, 5) + 0.015625f*O(7, 7));

        if (lmax < 8) return;
        vout[index++] = (-0.8356088723200586f*O(8, 1) + 0.516334738808072f*O(8, 3) -
            0.184877493221863f*O(8, 5) + 0.03125f*O(8, 7));
        vout[index++] = (-0.109375f*O(8, -7) + 0.4621937330546575f*O(8, -5) - 0.774502108212108f*O(8, -3) +
            0.4178044361600293f*O(8, -1));
        vout[index++] = (-0.4576818286211503f*O(8, 1) - 0.47134697278119864f*O(8, 3) +
            0.7088310138883598f*O(8, 5) - 0.2567449488305466f*O(8, 7));
        vout[index++] = (0.4621937330546575f*O(8, -7) - 0.703125f*O(8, -5) - 0.2181912506838897f*O(8, -3) +
            0.4943528756111367f*O(8, -1));
        vout[index++] = (-0.27421763710600383f*O(8, 1) - 0.6051536478449089f*O(8, 3) -
            0.33802043207474897f*O(8, 5) + 0.6665852814906732f*O(8, 7));
        vout[index++] = (-0.774502108212108f*O(8, -7) - 0.2181912506838897f*O(8, -5) +
            0.265625f*O(8, -3) + 0.5310201708739509f*O(8, -1));
        vout[index++] = (-0.1307281291459493f*O(8, 1) - 0.38081430021731066f*O(8, 3) -
            0.5908647000371574f*O(8, 5) - 0.6991205412874092f*O(8, 7));
        vout[index++] = (0.4178044361600293f*O(8, -7) + 0.4943528756111367f*O(8, -5) +
            0.5310201708739509f*O(8, -3) + 0.546875f*O(8, -1));
        vout[index++] = (0.2734375f*O(8, 0) + 0.3921843874378479f*O(8, 2) + 0.4113264556590057f*O(8, 4) +
            0.4576818286211503f*O(8, 6) + 0.626706654240044f*O(8, 8));
        vout[index++] = (0.8356088723200586f*O(8, -8) + 0.4576818286211503f*O(8, -6) +
            0.27421763710600383f*O(8, -4) + 0.1307281291459493f*O(8, -2));
        vout[index++] = (0.3921843874378479f*O(8, 0) + 0.5f*O(8, 2) + 0.32775276505317236f*O(8, 4) -
            0.6991205412874092f*O(8, 8));
        vout[index++] = (-0.516334738808072f*O(8, -8) + 0.47134697278119864f*O(8, -6) +
            0.6051536478449089f*O(8, -4) + 0.38081430021731066f*O(8, -2));
        vout[index++] = (0.4113264556590057f*O(8, 0) + 0.32775276505317236f*O(8, 2) -
            0.28125f*O(8, 4) - 0.7302075903467452f*O(8, 6) + 0.3332926407453366f*O(8, 8));
        vout[index++] = (0.184877493221863f*O(8, -8) - 0.7088310138883598f*O(8, -6) +
            0.33802043207474897f*O(8, -4) + 0.5908647000371574f*O(8, -2));
        vout[index++] = (0.4576818286211503f*O(8, 0) - 0.7302075903467452f*O(8, 4) + 0.5f*O(8, 6) -
            0.0855816496101822f*O(8, 8));
        vout[index++] = (-0.03125f*O(8, -8) + 0.2567449488305466f*O(8, -6) - 0.6665852814906732f*O(8, -4) +
            0.6991205412874092f*O(8, -2));
        vout[index++] = (0.626706654240044f*O(8, 0) - 0.6991205412874092f*O(8, 2) +
            0.3332926407453366f*O(8, 4) - 0.0855816496101822f*O(8, 6) + 0.0078125f*O(8, 8));

        if (lmax < 9) return;
        vout[index++] = (0.6090493921755238f*O(9, 0) - 0.6968469725305549f*O(9, 2) +
            0.3615761395439417f*O(9, 4) - 0.11158481919598204f*O(9, 6) + 0.016572815184059706f*O(9, 8));
        vout[index++] = (-0.0625f*O(9, -8) + 0.3156095293238149f*O(9, -6) - 0.6817945071647321f*O(9, -4) +
            0.656993626300895f*O(9, -2));
        vout[index++] = (0.44314852502786806f*O(9, 0) - 0.05633673867912483f*O(9, 2) - 0.6723290616859425f*O(9, 4) +
            0.5683291712335379f*O(9, 6) - 0.1594400908746762f*O(9, 8));
        vout[index++] = (0.3156095293238149f*O(9, -8) - 0.71875f*O(9, -6) + 0.20252314682524564f*O(9, -4) +
            0.5854685623498499f*O(9, -2));
        vout[index++] = (0.39636409043643195f*O(9, 0) + 0.25194555463432966f*O(9, 2) - 0.3921843874378479f*O(9, 4) -
            0.6051536478449089f*O(9, 6) + 0.509312687906457f*O(9, 8));
        vout[index++] = (-0.6817945071647321f*O(9, -8) + 0.20252314682524564f*O(9, -6) + 0.5625f*O(9, -4) +
            0.4215855488510013f*O(9, -2));
        vout[index++] = (0.3754879637718099f*O(9, 0) + 0.42961647140211f*O(9, 2) + 0.13799626353637262f*O(9, 4) -
            0.2981060004427955f*O(9, 6) - 0.7526807559068452f*O(9, 8));
        vout[index++] = (0.656993626300895f*O(9, -8) + 0.5854685623498499f*O(9, -6) + 0.4215855488510013f*O(9, -4) +
            0.21875f*O(9, -2));
        vout[index++] = (0.36685490255855924f*O(9, 0) + 0.5130142237306876f*O(9, 2) + 0.4943528756111367f*O(9, 4) +
            0.4576818286211503f*O(9, 6) + 0.38519665736315783f*O(9, 8));
        vout[index++] = (-0.6090493921755238f*O(9, -9) - 0.44314852502786806f*O(9, -7) - 0.39636409043643195f*O(9, -5) -
            0.3754879637718099f*O(9, -3) - 0.36685490255855924f*O(9, -1));
        vout[index++] = (0.0546875f*O(9, 1) + 0.16792332234534904f*O(9, 3) + 0.2954323500185787f*O(9, 5) +
            0.4624247721758373f*O(9, 7) + 0.8171255055356398f*O(9, 9));
        vout[index++] = (0.6968469725305549f*O(9, -9) + 0.05633673867912483f*O(9, -7) - 0.25194555463432966f*O(9, -5) -
            0.42961647140211f*O(9, -3) - 0.5130142237306876f*O(9, -1));
        vout[index++] = (0.16792332234534904f*O(9, 1) + 0.453125f*O(9, 3) + 0.577279787559724f*O(9, 5) +
            0.387251054106054f*O(9, 7) - 0.5322256665703469f*O(9, 9));
        vout[index++] = (-0.3615761395439417f*O(9, -9) + 0.6723290616859425f*O(9, -7) + 0.3921843874378479f*O(9, -5) -
            0.13799626353637262f*O(9, -3) - 0.4943528756111367f*O(9, -1));
        vout[index++] = (0.2954323500185787f*O(9, 1) + 0.577279787559724f*O(9, 3) + 0.140625f*O(9, 5) -
            0.7162405240429014f*O(9, 7) + 0.21608307321780204f*O(9, 9));
        vout[index++] = (0.11158481919598204f*O(9, -9) - 0.5683291712335379f*O(9, -7) + 0.6051536478449089f*O(9, -5) +
            0.2981060004427955f*O(9, -3) - 0.4576818286211503f*O(9, -1));
        vout[index++] = (0.4624247721758373f*O(9, 1) + 0.387251054106054f*O(9, 3) - 0.7162405240429014f*O(9, 5) +
            0.34765625f*O(9, 7) - 0.048317644050206957f*O(9, 9));
        vout[index++] = (-0.016572815184059706f*O(9, -9) + 0.1594400908746762f*O(9, -7) - 0.509312687906457f*O(9, -5) +
            0.7526807559068452f*O(9, -3) - 0.38519665736315783f*O(9, -1));
        vout[index++] = (0.8171255055356398f*O(9, 1) - 0.5322256665703469f*O(9, 3) + 0.21608307321780204f*O(9, 5) -
            0.048317644050206957f*O(9, 7) + 0.00390625f*O(9, 9));

    #undef O
    }

    void rotate_matrix_to_zyz(const glm::mat3 &m, float *alpha, float *beta, float *gamma) {
        #define M(a, b) (m[a][b])

            float sy = sqrtf(M(2, 1)*M(2, 1) + M(2, 0)*M(2, 0));
            if (sy > 16 * FLT_EPSILON) {
                *gamma = -atan2f(M(1, 2), -M(0, 2));
                *beta = -atan2f(sy, M(2, 2));
                *alpha = -atan2f(M(2, 1), M(2, 0));
            }
            else {
                *gamma = 0;
                *beta = -atan2f(sy, M(2, 2));
                *alpha = -atan2f(-M(1, 0), M(1, 1));
            }
        #undef M
    }

    float brdf(int inIndex, glm::vec3 V) {
        // Blinn-Phong BRDF
        glm::vec3 N( 0, 0, 1 );
        glm::vec3 L( sin(theta[inIndex]) * cos(phi[inIndex]), sin(theta[inIndex]) * sin(phi[inIndex]), cos(theta[inIndex]) );
        glm::vec3 H = glm::normalize(L + V);
        if (L.z < 0 || V.z < 0)
            return 0.f;
        return 0.5f + 3.5f * pow(fmax(0.f,glm::dot(N, H)), 8.f) / glm::dot(N, L);
    }    

    float brdf(int inIndex, int outIndex) {
        // Blinn-Phong BRDF
        
        
        glm::vec3 N( 0, 0, 1 );
        glm::vec3 L( sin(theta[inIndex]) * cos(phi[inIndex]), sin(theta[inIndex]) * sin(phi[inIndex]), cos(theta[inIndex]) );
        glm::vec3 V( sin(theta[outIndex]) * cos(phi[outIndex]), sin(theta[outIndex]) * sin(phi[outIndex]), cos(theta[outIndex]));
        glm::vec3 H = glm::normalize(L + V);
        if (L.z < 0 || V.z < 0)
            return 0.f;
        return 0.5f + 3.5f * pow(fmax(0.f,glm::dot(N, H)), 8.f) / glm::dot(N, L);
    }

    glm::vec3 calc_diffuse_color(Vertex vertex, glm::vec3 lightcolor = glm::vec3(1.0,1.0,1.0))
    {
        int lmaxlmax = (lmax + 1) * (lmax + 1);
        for (int j = 0; j < lmaxlmax; ++j) {
            light_coeff[j] = glm::vec3(0, 0, 0);
            transfer_coeff[j] = glm::vec3(0, 0, 0);
        }

        for (int j = 0; j < samps; ++j) {
            glm::vec3 lightdir(sin(theta[j]) * cos(phi[j]), sin(theta[j]) * sin(phi[j]), cos(theta[j]));
            int index;
            float u,v;
            convert_xyz_to_cube_uv(lightdir, &index, &u, &v);
            glm::vec3 color_value = lightcolor * lamptexture[index]->get_color_uv(u,v);
            
            
            // calc light
            for (int k = 0; k < lmaxlmax; ++k) {
                light_coeff[k] += color_value * y_coeff[j][k];
            }
            // calc transfer

            glm::vec3 dir(sin(theta[j]) * cos(phi[j]), sin(theta[j]) * sin(phi[j]), cos(theta[j]));

            if(!bvhTree.ray_intersect_with_mesh(BvhTree::Ray(vertex.Position,dir)))
            {
                float cos_value = glm::dot(
                    vertex.Normal,
                    glm::vec3(sin(theta[j]) * cos(phi[j]), sin(theta[j]) * sin(phi[j]), cos(theta[j]))
                );
                
                for (int k = 0; k < lmaxlmax; ++k) {
                    float single = std::max(cos_value, 0.f) * y_coeff[j][k];
                    transfer_coeff[k] += glm::vec3(single, single, single);
                }
            }
        }
        glm::vec3 color(0, 0, 0);
        for (int j = 0; j < lmaxlmax; ++j) {
            light_coeff[j] *= (4.f * M_PI) / float(samps);
            transfer_coeff[j] *= (4.f * M_PI) / float(samps);
            color += light_coeff[j] * transfer_coeff[j] / float(M_PI);
        }
        return color;
    }


    glm::vec3 calc_glossy_color(Vertex vertex, glm::vec3 lightcolor = glm::vec3(5.0,5.0,5.0))
    {
        std::vector<glm::vec3> light, shadow, local, reflect;

        int lmaxlmax = (lmax + 1) * (lmax + 1);

        // calc vector (light)
        for (int j = 0; j < lmaxlmax; ++j) {
            glm::vec3 light_coeff( 0, 0, 0 );
            for (int k = 0; k < samps; ++k) {
                glm::vec3 lightdir(sin(theta[k]) * cos(phi[k]), sin(theta[k]) * sin(phi[k]), cos(theta[k]));
                int index;
                float u,v;
                convert_xyz_to_cube_uv(lightdir, &index, &u, &v);
                glm::vec3 color_value = lightcolor * lamptexture[index]->get_color_uv(u,v);

                light_coeff += color_value * y_coeff[k][j];
            }
            light.push_back(light_coeff * float(4.f * M_PI) / float(samps));
        }

        // clac T * vector (shadow)
        for (int j = 0; j < lmaxlmax; ++j)
            shadow.push_back(glm::vec3(0, 0, 0));
        for (int k = 0; k < samps; ++k) {
            glm::vec3 dir(sin(theta[k]) * cos(phi[k]), sin(theta[k]) * sin(phi[k]), cos(theta[k]));
            if(!bvhTree.ray_intersect_with_mesh(BvhTree::Ray(vertex.Position,dir)))
            {
                for (int j = 0; j < lmaxlmax; ++j)
                    for (int p = 0; p < lmaxlmax; ++p)
                        shadow[j] += light[p] * y_coeff[k][j] * y_coeff[k][p];
            }
        }
        for (int j = 0; j < lmaxlmax; ++j)
            shadow[j] *= (4.f * M_PI) / float(samps);

        // calc R * T * vector (local)
        glm::vec3 zcz = glm::cross(glm::vec3(0, 0, 1), vertex.Normal);
        float zdz = glm::dot(glm::vec3(0, 0, 1), vertex.Normal);
        float alpha = atan2(zcz.y, zcz.x) - M_PI * 0.5f;
        float beta = acos(zdz);
        float gamma = 0;
        
        glm::mat3 m1( cos(alpha),sin(alpha),0,-sin(alpha),cos(alpha),0,0,0,1 );
        glm::mat3 m2( cos(beta),0,-sin(beta),0,1,0,sin(beta),0,cos(beta) );
        glm::mat3 m3( cos(gamma),sin(gamma),0,-sin(gamma),cos(gamma),0,0,0,1 );
        glm::mat3 rotate = m1 * m2 * m3;
        rotate_matrix_to_zyz(rotate, &alpha, &beta, &gamma);
        
        std::vector<glm::vec3> tmp1, tmp2, tmp3, tmp4;
        rotate_z(shadow, tmp1, -alpha);
        rotate_x_plus(tmp1, tmp2);
        rotate_z(tmp2, tmp3, -beta);
        rotate_x_minus(tmp3, tmp4);
        rotate_z(tmp4, local, -gamma);
        
        // calc B * R * T * vector (reflection)
        
        
        for (int j = 0; j < lmaxlmax; ++j) {
            glm::vec3 reflect_coeff = glm::vec3(0,0,0);
            for (int k = 0; k < lmaxlmax; ++k)
                for (int p = 0; p < samps; ++p)
                    for (int q = 0; q < samps; ++q)
                        reflect_coeff += local[k] * fabs(cos(theta[p])) * y_coeff[p][k] * y_coeff[q][j] * brdf(p, q);
            reflect.push_back(reflect_coeff * float(4.f * M_PI) / float(samps) * float(4.f * M_PI) / float(samps));
        }

        // calc one point color
        glm::vec3 dir = glm::normalize(camera.Position - vertex.Position);
        dir = rotate * dir;
        std::vector<float> yy;
        calc_sh_coeff(acos(dir.z),atan2(dir.y, dir.x),yy);
        glm::vec3 color(0, 0, 0);
        for (int j = 0; j < lmaxlmax; ++j)
            color += yy[j] * reflect[j] / float(M_PI);
        
        return color;
        
    
    }

public:
	Prt(Camera camera)
    {
        srand((int)time(0));
        generate_sample_angles();
        generate_sh();

        this->camera = camera;
        
        modelshader.load("/Users/apple/Desktop/myprt/shader/model.vs", "/Users/apple/Desktop/myprt/shader/model.frag");
        //boxshader.load("/Users/apple/Desktop/myprt/shader/envmap.vs", "/Users/apple/Desktop/myprt/shader/envmap.frag");
        boxshader.load("/Users/apple/Desktop/myprt/shader/skybox.vs", "/Users/apple/Desktop/myprt/shader/skybox.frag");
        ourModel.loadModel("/Users/apple/Desktop/myprt/obj/nanosuit/nanosuit.obj");
        //ourModel.loadModel("/Users/apple/Desktop/myprt/obj/buddha/buddha.obj");
        //ourModel.loadModel("/Users/apple/Desktop/myprt/obj/suzanne/suzanne.obj");
        ourFloor.loadModel("/Users/apple/Desktop/myprt/obj/floor/floor.obj");
        bvhTree.load(ourModel);
        bvhTree.load(ourFloor);
        bvhTree.tobuild();

        faces.push_back("/Users/apple/Desktop/myprt/texture/skybox/right.jpg");
        faces.push_back("/Users/apple/Desktop/myprt/texture/skybox/left.jpg");
        faces.push_back("/Users/apple/Desktop/myprt/texture/skybox/top.jpg");
        faces.push_back("/Users/apple/Desktop/myprt/texture/skybox/bottom.jpg");
        faces.push_back("/Users/apple/Desktop/myprt/texture/skybox/back.jpg");
        faces.push_back("/Users/apple/Desktop/myprt/texture/skybox/front.jpg");
        for(int i = 0; i < 6; i++)
        {
            lamptexture[i] = new LampTexture(faces[i]);
        }
    }
    

	void prepare()
    {
        
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glBindVertexArray(0);
        
        cubemapTexture = loadCubemap(faces);
        
        std::cout<<ourModel.meshes.size()<<std::endl;
        for(int i = 0; i < ourModel.meshes.size(); i++)
        {
            Mesh& mesh = ourModel.meshes[i];
            std::cout<<i<<" "<<mesh.vertices.size() << std::endl;
            for(int j = 0; j < mesh.vertices.size(); j++)
            {
                std::cout<<j<<std::endl;
                mesh.vertices[j].Prtcolor = calc_diffuse_color(mesh.vertices[j], glm::vec3(5.0,5.0,5.0));
            }
            mesh.setup();
        }

        
        for(int i = 0; i < ourFloor.meshes.size(); i++)
        {
            Mesh& mesh = ourFloor.meshes[i];
            for(int j = 0; j < mesh.vertices.size(); j++)
            {
                std::cout<<j<< " "<< mesh.vertices.size()<<std::endl;
                mesh.vertices[j].Prtcolor = calc_diffuse_color(mesh.vertices[j], glm::vec3(0.6,0.6,0.6));
            }
            mesh.setup();
        }
        
    }

	void render(Camera camera)
    {
        // Draw scene as normal
        modelshader.Use();
        glm::mat4 model;
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(modelshader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(modelshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(modelshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        ourModel.Draw(modelshader);
        ourFloor.Draw(modelshader);
        
        // Draw skybox as last
        glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
        boxshader.Use();
        
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
        glUniformMatrix4fv(glGetUniformLocation(boxshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(boxshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(boxshader.Program, "skybox"), 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        
        glDepthFunc(GL_LESS); // Set depth function back to default
    }

};


#endif
