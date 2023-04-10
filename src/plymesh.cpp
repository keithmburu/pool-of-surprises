//--------------------------------------------------
// Author: Keith Mburu
// Date: 2/25/2023
// Description: Loads PLY files in ASCII format
//--------------------------------------------------

#include "plymesh.h"
#include <fstream>

using namespace std;
using namespace glm;

namespace agl {

   PLYMesh::PLYMesh(const string& filename) {
      load(filename);
   }

   PLYMesh::PLYMesh() {
   }

   void PLYMesh::init() {
      assert(_positions.size() != 0);
      initBuffers(&_faces, &_positions, &_normals, &_uvs);
   }

   PLYMesh::~PLYMesh() {
   }

   bool PLYMesh::load(const string& filename) {
      if (_positions.size() != 0) {
         std::cout << "WARNING: Cannot load different files with the same PLY mesh\n";
         return false;
      }
      std::ifstream file(filename);
      if (!file) {
         return false;
      } else {
         string buffer;
         std::getline(file, buffer);
         if (buffer != "ply") {
            return false;
         } else {
            int numVertices, numPolygons;
            int numVerticesIdx = 15, numPolygonsIdx = 13;
            while (std::getline(file, buffer)) {
               if (buffer.find("element vertex") != string::npos) {
                  numVertices = std::stoi(buffer.substr(numVerticesIdx));
               } else if (buffer.find("element face") != string::npos) {
                  // cout << buffer << endl;
                  numPolygons = std::stoi(buffer.substr(numPolygonsIdx));
               } else if (buffer.find("end_header") != string::npos) {
                  // cout << buffer << endl;
                  break;
               }
            }
            for (int i = numVertices; i > 0; i--) {
               std::getline(file, buffer);
               // cout << buffer << endl;
               int begin = 0;
               int end = buffer.find(" ", begin);
               for (int j = 1; j <= 11; j++) {
                  // cout << begin << " " << end << " " << buffer.substr(begin, 8) << endl;
                  if (j <= 3) {
                     _positions.push_back(stof(buffer.substr(begin, end - begin)));
                  } else if (j <= 6) {
                     _normals.push_back(stof(buffer.substr(begin, end - begin)));
                  } else if (j <= 8) {
                     _uvs.push_back(stof(buffer.substr(begin, end - begin)));
                  } else if (j <= 11) {
                     _colors.push_back(stof(buffer.substr(begin, end - begin)));
                  }
                  if (end == -1) break;
                  begin = end + 1;
                  end = buffer.find(" ", begin);
               }
            }
            for (int i = numPolygons; i > 0; i--) {
               std::getline(file, buffer);
               // cout << buffer << endl;
               int begin = 0;
               int end = buffer.find(" ", begin);
               int vertexCount = 0;
               for (int j = 1; j <= 4; j++) {
                  // cout << begin << " " << end << " " << buffer.substr(begin, 1) << endl;
                  if (!vertexCount) {
                     vertexCount = stoi(buffer.substr(begin, end - begin));
                  } else {
                     _faces.push_back(stoi(buffer.substr(begin, end - begin)));
                  }
                  begin = end + 1;
                  end = buffer.find(" ", begin);
               }
            }
            return true;
         }
      }
   }

   glm::vec3 PLYMesh::minBounds() const {
      float minX, minY, minZ;
      minX = minY = minZ = 999999;
      for (int i = 0; i < _positions.size(); i += 3) {
         minX = std::min(minX, _positions[i]);
         minY = std::min(minY, _positions[i + 1]);
         minZ = std::min(minZ, _positions[i + 2]);
      }
      return glm::vec3(minX, minY, minZ);
   }

   glm::vec3 PLYMesh::maxBounds() const { 
      float maxX, maxY, maxZ;
      maxX = maxY = maxZ = 0;
      for (int i = 0; i < _positions.size(); i += 3) {
         maxX = std::max(maxX, _positions[i]);
         maxY = std::max(maxY, _positions[i + 1]);
         maxZ = std::max(maxZ, _positions[i + 2]);
      }
      return glm::vec3(maxX, maxY, maxZ);
   }

   int PLYMesh::numVertices() const {
      return _positions.size() / 3;
   }

   int PLYMesh::numTriangles() const {
      return _faces.size() / 3;
   }

   const vector<GLfloat>& PLYMesh::positions() const {
      return _positions;
   }

   const vector<GLfloat>& PLYMesh::normals() const {
      return _normals;
   }

   const vector<GLuint>& PLYMesh::indices() const {
      return _faces;
   }

   const vector<GLfloat>& PLYMesh::colors() const {
      return _colors;
   }

   const vector<GLfloat>& PLYMesh::uvs() const {
      return _uvs;
   }

   bool PLYMesh::hasUV() const {
      return (_uvs.size() != 0);
   }

}
