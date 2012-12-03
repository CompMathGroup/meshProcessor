#ifndef __MESH3D__MESH_H__
#define __MESH3D__MESH_H__

#include <cmath>

#include "vector.h"

#include <vector>
#include <string>
#include <fstream>

enum ElementType /*: int*/ {
	EL_TETRAHEDRON = 1,
	EL_TYPE_END
};

enum FaceType /*: int*/ {
	FC_TRIANGLE = 1,
	FC_TYPE_END
};

struct Element;
struct Face;

struct Vertex {
	int index;
	Vector r;
	std::vector<std::pair<Element *, int> > elems; // List of elements and indices with this vertex
	std::vector<std::pair<Face *, int> > bnds; // List of boundary faces and 

	Vertex(int _index, const Vector &v): index(_index), r(v) { }
private:
	Vertex &operator=(const Vertex &p);
	Vertex(const Vertex &p);
};

struct Element {
	int index;
	Vector center;
	double volume;
	ElementType type;
	int region;
	double quality;

	virtual ~Element() {}
protected:
	Element(int _index) : index(_index) {}
private:
	Element();
	Element(const Element &);
};

struct Face {
	int index;
	int bnd_index;
	Element *element;
	Face *flip;

	FaceType type;
	int borderType;
	Vector normal;
	Vector center;
	double surface;	
	virtual ~Face() {}
	virtual void setFlip(Face *_flip) {
		flip = _flip;
	}
protected:
	Face(int _index, Element *_element) : index(_index), element(_element) {}
private:
	Face();
	Face(const Face &);
};

class Mesh {
	void fillData(
		std::vector<std::vector<int> > &vertex2face,
		int nV, int nB, int nT, 
		double *vert, int *bnd, int *tet, 
		int *bndmat, int *tetmat);
	void fromVol(
		std::ostream &log,
		int nV, int nB, int nT, 
		double *vert, int *bnd, int *tet, 
		int *bndmat, int *tetmat);
	bool checkVertexIndex(int &wrong) const;
	bool checkFaceIndex(int &wrong) const;
	bool checkFlippedFace(int &wrong) const;
	bool checkFlippedOrient(int &wrong) const;
	bool checkFaceBoundaryCondidion(int &wrong, bool &isOuter) const;
	bool checkFaceSurface(int &wrong) const;
	bool checkElementVolume(int &wrong) const;
	bool checkElementFaceNormals(int &wrong, int &face) const;
	bool checkVertexBoundaryList(
		int &wrong, int &faceno, int &vertno, int &cnt) const;
	bool checkVertexElementsList(
		int &wrong, int &tetno, int &vertno, int &cnt) const;
public:
	int nVert, nFaces, nElems, nBndFaces;
	std::vector<Vertex *> vertices;
	std::vector<Element *> elements;
	std::vector<Face *> faces;

	Mesh(std::ostream &log, const char *fn);
	void saveVtk(
		std::ostream &log,
		const char *fn, int realbytes, 
		const char *cellfmt, const char *pointfmt, ...) const;
	bool check(std::ostream &log) const;
	double quality() const;
	~Mesh();
};

#endif 
