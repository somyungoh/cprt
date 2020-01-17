#ifndef MESH_H
#define MESH_H
/************************************************************
 * 															*
 * 		mesh.h		                                	    *
 * 															*
 * 		Declaration of the mesh class     				    *
 * 															*
 ************************************************************/


#include <GL/glew.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"

#include <vector>
#include <map>

// *****	Base Mesh Class    ***** //

class Mesh {
public:
    /*  Mesh Data  */
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
    glm::mat4 model;    // model transform
    unsigned int VAO;

    /*  Functions  */
    // constructor
	Mesh();
	virtual ~Mesh();

    // render the mesh
    virtual void draw(GLenum DRAW_MODE, Shader &shader, const glm::mat4 &model, const glm::vec3 &color) const;

protected:
    /*  Render data  */
    GLuint VBO, EBO;

    /*  Functions    */
    // initializes all the buffer objects/arrays
    virtual void setupMesh();
};


class Cube : public Mesh {
public:
	Cube(float scale);
	virtual ~Cube();

    // render the mesh
    void draw(GLenum DRAW_MODE, Shader &shader, const glm::mat4 &model, const glm::vec3 &color) const override;

private:
	float scale;
	virtual void setupMesh();
};

class Icosphere : public Mesh {
	//GLuint positionBuffer;
	//GLuint texcoordsBuffer;
	GLuint normalBuffer;
	GLuint indexBuffer;
	size_t numVertices;
	size_t numIndices;

public:
    // ctor/dtor
    Icosphere(float radius=1.0f, int subdivision=1, bool smooth=false);
    virtual ~Icosphere();

    // getters/setters
    float getRadius() const                 { return radius; }
    void setRadius(float radius);
    int getSubdivision() const              { return subdivision; }
    void setSubdivision(int subdivision);
    bool getSmooth() const                  { return smooth; }
    void setSmooth(bool smooth);

    // draw in VertexArray mode
    void draw(GLenum DRAW_MODE, Shader &shader, const glm::mat4 &model, const glm::vec3 &color) const override;
    
private:
    // static functions
    static void computeFaceNormal(const float v1[3], const float v2[3], const float v3[3], float normal[3]);
    static void computeVertexNormal(const float v[3], float normal[3]);
    static float computeScaleForLength(const float v[3], float length);
    static void computeHalfVertex(const float v1[3], const float v2[3], float length, float newV[3]);
    static void computeHalfTexCoord(const float t1[2], const float t2[2], float newT[2]);
    static bool isSharedTexCoord(const float t[2]);
    static bool isOnLineSegment(const float a[2], const float b[2], const float c[2]);

    // member functions
    void updateRadius();
    std::vector<float> computeIcosahedronVertices();
    void buildVerticesFlat();
    void buildVerticesSmooth();
    void subdivideVerticesFlat();
    void subdivideVerticesSmooth();
    void buildInterleavedVertices();
    void addVertex(float x, float y, float z);
    void addVertices(const float v1[3], const float v2[3], const float v3[3]);
    void addNormal(float nx, float ny, float nz);
    void addNormals(const float n1[3], const float n2[3], const float n3[3]);
    void addTexCoord(float s, float t);
    void addTexCoords(const float t1[2], const float t2[2], const float t3[2]);
    void addIndices(unsigned int i1, unsigned int i2, unsigned int i3);
    void addSubLineIndices(unsigned int i1, unsigned int i2, unsigned int i3,
                           unsigned int i4, unsigned int i5, unsigned int i6);
    unsigned int addSubVertexAttribs(const float v[3], const float n[3], const float t[2]);

    // memeber vars
    float radius;                           // circumscribed radius
    int subdivision;
    bool smooth;
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> lineIndices;
    std::map<std::pair<float, float>, unsigned int> sharedIndices;   // indices of shared vertices, key is tex coord (s,t)

    // interleaved
    std::vector<float> interleavedVertices;
    int interleavedStride;                  // # of bytes to hop to the next vertex (should be 32 bytes)
};

#endif