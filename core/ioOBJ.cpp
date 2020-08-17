// Copyright (c) 2007,2010, Eduard Heidt

#include "SceneIO.h"

#include <string>
#include <unordered_map>
#include <fstream>
#include <array>

#include <vector>
#include <string>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <string.h>

namespace eh{

typedef SceneIO::progress_callback progress_callback;

class OBJLoader
{
private:

	std::vector<Vec3> m_vertices;
	std::vector<Vec3> m_normals;
	std::vector<Vec3> m_texcoords;

	progress_callback progress;

	Ptr<IVertexBuffer> m_pVB;

	inline Uint addVNT(size_t vi, size_t ni, size_t ti)
	{
		if(vi < m_vertices.size() && ni < m_normals.size() && ti < m_texcoords.size())
			return m_pVB->addVertex( m_vertices[vi], m_normals[ni], m_texcoords[ti] );
		else if(vi < m_vertices.size() && ni < m_normals.size())
			return m_pVB->addVertex( m_vertices[vi], m_normals[ni], Vec3::Null() );
		else if(vi < m_vertices.size())
			return m_pVB->addVertex( m_vertices[vi], Vec3::Null(), Vec3::Null() );
		else
			return 0;
	}

	typedef std::unordered_map< std::string, std::unordered_map< std::string, Uint_vec> > FaceMap;

	std::unordered_map< std::string, Ptr<Material> > m_materials;
	std::string m_current_matereal;
	std::string m_current_object;
	FaceMap m_faces;
	std::unordered_map<std::string, Uint_vec> m_edges;

	void addVertex(const std::string& line)
	{
		Float x,y,z;
		std::stringstream(&line[2]) >> x >> y >> z;
		m_vertices.push_back(Vec3(x,y,z));
	}

	void addNormal(const std::string& line)
	{
		Float x,y,z;
		std::stringstream(&line[3]) >> x >> y >> z;
		m_normals.push_back(Vec3(x,y,z));
	}

	void addTexCoord(const std::string& line)
	{
		Float u,v;
		std::stringstream(&line[3]) >> u >> v;
		m_texcoords.push_back(Vec3(u,v,0));
	}

	bool addMaterial(const std::string& line)
	{
		RGBA rgba(0,0,0,0);
		if(sscanf(line.c_str(), "#RGBA %f %f %f %f", &rgba.r, &rgba.g, &rgba.b, &rgba.a) != 4)
			return false;

		if(m_materials.find(m_current_matereal) == m_materials.end())
			m_materials[m_current_matereal] = Material::create(rgba);

		return true;
	}

	void useMaterial(const std::string& line)
	{
		char buffer[256];
		if(sscanf(line.c_str(), "usemtl %s", buffer) == 1)
			m_current_matereal = buffer;
	}

	void setObject(const std::string& line)
	{
		char buffer[256];
		if(sscanf(line.c_str(), "o %s", buffer) == 1)
			m_current_object = buffer;
	}

	bool makeFace(const std::string& line);
	bool makeLine(const std::string& line);
	bool loadMaterials(const std::string& line);

public:
	OBJLoader(): m_pVB( nullptr )
	{
	}
	virtual ~OBJLoader()
	{}
	bool read(const std::wstring& sFile, SceneNodeVector& nodes, progress_callback progress)
	{
		SceneIO::File aFile(sFile);

		std::unique_ptr<char> data;
		size_t size = aFile.getContent(data);

		if(size == 0)
			return false;

		std::stringstream   file;
		file.write(data.get(), size); //Todo: Optimize
		file.seekg(0);

		//file.rdbuf()->pubsetbuf(data.get(), size);
		//std::iostreams::stream<boost::iostreams::array_source>  file(data.get(), size);

		//boost::filesystem::ifstream file( sFile.c_str());

		//if(!file.is_open())
		//	return false;

		bool ret = read(file, nodes, progress);

		//file.close();

		return ret;
	}

	bool read(std::istream& stream, SceneNodeVector& nodes, progress_callback progress)
	{
		m_pVB = CreateVertexBuffer( sizeof(Vec3)*2 + sizeof(Float)*2);

		float nCount = 0;
		float iCount = 0;


		std::string line;
		while( !stream.eof() )
		{
			nCount+=1;
			std::getline(stream, line);
		}

		stream.clear();
		stream.seekg(0, std::ios_base::beg );

		while( !stream.eof() )
		{
			progress(++iCount/nCount);
			std::getline(stream, line);

			if(!strncmp("#RGBA", line.c_str(), 5))		addMaterial(line);	//SPECIAL TAG, not OBJ Specific
			else if(!strncmp("vt", line.c_str(), 2))	addTexCoord(line);
			else if(!strncmp("vn", line.c_str(), 2))	addNormal(line);
			else if(!strncmp("v", line.c_str(), 1))		addVertex(line);
			else if(!strncmp("f", line.c_str(), 1))		makeFace(line);
			else if(!strncmp("l", line.c_str(), 1))		makeLine(line);
			else if(!strncmp("mtllib", line.c_str(), 6))	loadMaterials(line);
			else if(!strncmp("usemtl", line.c_str(), 6))	useMaterial(line);
			else if(!strncmp("o", line.c_str(), 1))		setObject(line);
			//else if(!strncmp("#", line.c_str(), 1))		; // ignore
			//else if(!strncmp("s", line.c_str(), 1))		; // ignore
		}

		if(m_faces.size() == 0 || m_pVB->getVertexCount() == 0)
		{
			Uint_vec indices;
			for(size_t i = 0; i < m_vertices.size(); i++)
				indices.push_back( m_pVB->addVertex( m_vertices[i] ) );

			nodes.push_back( ShapeNode::create( Material::Black(),Geometry::create( Geometry::POINTS, m_pVB, indices )  ) );
		}
		else
		{

			for(FaceMap::const_iterator it = m_faces.begin(); it != m_faces.end(); ++it)
			{
				Ptr<ShapeNode> aShape = ShapeNode::create();
				for(auto it2 = it->second.begin();
					it2 != it->second.end(); ++it2)
					aShape->addGeometry( m_materials[it2->first], Geometry::create(Geometry::TRIANGLES, m_pVB, it2->second) );

				if(m_edges[it->first].size() > 0)
					aShape->addGeometry( Material::Black(), Geometry::create(Geometry::LINES, m_pVB, m_edges[it->first]) );

				nodes.push_back( aShape );
			}
		}

		std::cout << "OBJ loaded.. " << m_faces.size() << " Materials, " << m_pVB->getVertexCount() << " Vertices" << std::endl;
		m_pVB = nullptr;

		return true;
	}
};


bool OBJLoader::makeFace(const std::string& line)
{
	Uint_vec& face = m_faces[m_current_object][m_current_matereal];

	int v[4];
	int n[4];
	int t[4];

	int r = 0;
	if( (r = sscanf(line.c_str(), "f %d//%d %d//%d %d//%d %d//%d",
		&v[0], &n[0], &v[1], &n[1], &v[2], &n[2], &v[3], &n[3])) >= 6)
	{
		for(int i = 0; i < r/2; i++)
		{
			if(v[i] < 0) v[i] = m_vertices.size()+v[i]+1;
			if(n[i] < 0) n[i] = m_normals.size()+n[i]+1;

			if( i < 3 )
				face.push_back( addVNT(v[i]-1, n[i]-1, 0) );
			else
			{
				face.push_back( face[face.size()-3] );
				face.push_back( face[face.size()-2] );
				face.push_back( addVNT(v[3]-1, n[3]-1, 0) );
			}
		}
	}
	else if( (r = sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
		&v[0], &t[0], &n[0], &v[1], &t[1], &n[1], &v[2], &t[2], &n[2], &v[3], &t[3], &n[3])) >= 9)
	{
		for(int i = 0; i < r/3; i++)
		{
			if(v[i] < 0) v[i] = m_vertices.size()+v[i]+1;
			if(n[i] < 0) n[i] = m_normals.size()+n[i]+1;
			if(t[i] < 0) t[i] = m_texcoords.size()+t[i]+1;

			if( i < 3 )
				face.push_back( addVNT(v[i]-1, n[i]-1, t[i]-1) );
			else
			{
				face.push_back( face[face.size()-3] );
				face.push_back( face[face.size()-2] );
				face.push_back( addVNT(v[3]-1, n[3]-1, t[3]-1) );
			}
		}
	}
	else if( (r = sscanf(line.c_str(), "f %d/%d %d/%d %d/%d %d/%d",
		&v[0], &t[0], &v[1], &t[1], &v[2], &t[2], &v[3], &t[3])) >= 6)
	{

		for(int i = 0; i < r/2; i++)
		{
			if(v[i] < 0) v[i] = m_vertices.size()+v[i]+1;
			if(t[i] < 0) t[i] = m_texcoords.size()+t[i]+1;
		}

		Vec3 n = calcNormal(m_vertices[v[0]-1], m_vertices[v[1]-1], m_vertices[v[2]-1]);

		for(int i = 0; i < r/2; i++)
		{
			if( i < 3 )
				face.push_back( m_pVB->addVertex( m_vertices[v[i]-1], n, m_texcoords[t[i]-1] ) );
			else
			{
				face.push_back( face[face.size()-3] );
				face.push_back( face[face.size()-2] );
				face.push_back( m_pVB->addVertex( m_vertices[v[3]-1], n, m_texcoords[t[3]-1] ) );
			}
		}
	}
	else if( (r = sscanf(line.c_str(), "f %d %d %d %d", &v[0], &v[1], &v[2], &v[3])) >= 3)
	{

		for(int i = 0; i < r; i++)
		{
			if(v[i] < 0) v[i] = m_vertices.size()+v[i]+1;
		}

		Vec3 n = calcNormal(m_vertices[v[0]-1], m_vertices[v[1]-1], m_vertices[v[2]-1]);

		for(int i = 0; i < r; i++)
		{
			if( i < 3 )
				face.push_back( m_pVB->addVertex( m_vertices[v[i]-1], n, Vec3::Null() ) );
			else
			{
				face.push_back( face[face.size()-3] );
				face.push_back( face[face.size()-2] );
				face.push_back( m_pVB->addVertex( m_vertices[v[3]-1], n, Vec3::Null() ) );
			}
		}
	}
	else
		std::cerr << "error in " << __FUNCTION__ << std::endl;
	//else if( sscanf(buf, "%d", &v) == 1 )
	//{
	//	face.push_back( m_pVB->addVertex( m_vertices[v-1], m_normals.size()>0?m_normals[v-1]:Vec3::Null(), Vec3::Null() ) );
	//
	//	while(fscanf(file, "%d", &v) == 1)
	//		face.push_back( m_pVB->addVertex( m_vertices[v-1], m_normals.size()>0?m_normals[v-1]:Vec3::Null(), Vec3::Null() ) );

	//}

	return true;
}

bool OBJLoader::makeLine(const std::string& line)
{
	int a = 0, b = 0;

	Uint_vec& edges = m_edges[m_current_object];

	if(sscanf(&line[0], "l %d// %d//", &a, &b) == 2)
	{
		if(a < 0) a = m_vertices.size()+a+1;
		if(b < 0) b = m_vertices.size()+b+1;

		math3D::Uint ia = addVNT(a-1, 0, 0);
		edges.push_back( ia );

		math3D::Uint ib = addVNT(b-1, 0, 0);
		edges.push_back( ib );
	}

	return true;
}

bool OBJLoader::loadMaterials(const std::string& _line)
{
	char buffer[256];
	sscanf(_line.c_str(), "mtllib %s", buffer);

	SceneIO::File aFile( buffer );

	std::unique_ptr<char> data;
	size_t size = aFile.getContent(data);

	std::stringstream   file;
	file.rdbuf()->pubsetbuf(data.get(), size);
	//boost::iostreams::stream<boost::iostreams::array_source>  file(data.get(), size);


	//boost::filesystem::ifstream file( abs_path( buffer ).c_str() );

	//if(!file.is_open())
	//	return false;

	Ptr<Material> pMat = nullptr;
	float r = 0, g = 0, b = 0, a = 0;

	std::string line;
	while( !file.eof() )
	{
		std::getline(file, line);

		boost::erase_all(line, "\r");
		boost::erase_all(line, "\n");
		boost::erase_all(line, "\t");
		//boost::trim_if(line, boost::is_any_of("\t\r\n") );

		if(sscanf(&line[0], "newmtl %s", buffer) > 0)
		{
			pMat = m_materials[buffer] = Material::create();
			r = 0, g = 0, b = 0, a = 0;
		}

		if(pMat == nullptr)
			continue;

		if(sscanf(&line[0], "map_Kd %s", buffer) > 0)
		{
			pMat->setTexture( SceneIO::createTexture(&line[7]) );
		}
		else if(sscanf(&line[0], "map_Ka %s", buffer) > 0)
		{
			pMat->setReflTexture( SceneIO::createTexture(&line[7]) );
		}
		else if(sscanf(&line[0], "map_D %s", buffer) > 0)
		{
			pMat->setOpacTexture( SceneIO::createTexture(&line[6]) );
		}
		else if(sscanf(&line[0], "map_bump %s", buffer) > 0)
		{
			pMat->setBumpTexture( SceneIO::createTexture(&line[9]) );
		}
		else if(!strncmp("Kd", line.c_str(), 2)) //sscanf(&line[0], "Kd %f %f %f", &r,&g,&b) > 0)
		{
			RGBA rgba = pMat->getDiffuse();
			std::stringstream(&line[3]) >> rgba.r >> rgba.g >> rgba.b;
			pMat->setDiffuse( rgba );
		}
		else if(sscanf(&line[0], "Ks %f %f %f", &r,&g,&b) > 0)
		{
			pMat->setSpecular( RGBA(r, g, b) );
		}
		else if(!strncmp("d", line.c_str(), 1)) //sscanf(&line[0], "d %f", &a) > 0)
		{
			RGBA rgba = pMat->getDiffuse();
			std::stringstream(&line[2]) >> rgba.a;
			rgba.a = 1-rgba.a;
			pMat->setDiffuse( rgba );
		}
	}

	//file.close();

	return true;
}

template< class T >
class NoDubVector
{
	typedef T Type;

	struct T_hash
	{
		std::size_t operator () (const Type & v) const
		{
			size_t seed = 0;

			std::hash<char> hasher;
			for (unsigned i = 0; i < sizeof(Type); i++)
				seed ^= hasher(*(((char*)this) + i)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

			return seed;
		}
	};

	typedef std::unordered_map< Type, unsigned int, T_hash > map;
	typedef std::vector< Type > vec;
	map m_idx;
	vec m_Buff;
public:
	NoDubVector(){};
	virtual ~NoDubVector(){};

	unsigned int insert(const Type& v)
	{
#if defined(_MSC_VER)
		auto it = m_idx.find( v );
		if(it != m_idx.end())
			return it->second;
#else
		if(m_idx.find( v ) != m_idx.end())
			return m_idx[v];
#endif
		m_Buff.push_back(v);
		m_idx[ m_Buff.back() ] = (unsigned int) m_Buff.size()-1;
		return (unsigned int) m_Buff.size()-1;
	}

	const void* getBuffer() const { return &m_Buff[0]; }
	unsigned int size() const { return (unsigned int)m_Buff.size(); }

	const std::vector< Type >& vector() const { return m_Buff; }

public:
	const Type& operator[](unsigned int i) const
	{
		return m_Buff[i];
	}
	typedef std::vector< Type > const_iterator;
	const_iterator begin() const { return m_Buff.begin(); }
	const_iterator end() const { return m_Buff.end(); }
};

class OBJExport: public IVisitor
{
	math3D::Matrix::Stack matstack;

	std::string m_sMTLfile;

	NoDubVector< RGBA > m_mat;
	Uint m_curmat;

	NoDubVector< Vec3 > m_v;
	NoDubVector< Vec3 > m_n;
	NoDubVector< Vec3 > m_t;

	struct fi
	{
		fi(Uint _v, Uint _n, Uint _t):v(_v), n(_n), t(_t){}
		Uint v;
		Uint n;
		Uint t;
	};

	std::unordered_map< Uint, std::vector< fi > > m_f;
	std::unordered_map< Uint, std::vector< Uint > > m_l;

public:
	OBJExport()
	{
	}

	virtual ~OBJExport()
	{
	}

	virtual void visit(Geometry& node)
	{
		Ptr<IVertexBuffer> vb = node.getVertexBuffer();

		Uint_vec indices = node.getIndices();

		if(indices.size() == 0)
			for(size_t i = 0; i < node.getVertexBuffer()->getVertexCount(); i++)
				indices.push_back(i);;

		switch(node.getType())
		{
		case Geometry::LINES:
			for(size_t i = 0; i < indices.size(); i++)
			{
				m_l[m_curmat].push_back( m_v.insert( transform(vb->getCoord( indices[i] ), matstack.getTop()) )+1 );
			}
			break;
		case Geometry::TRIANGLES:
			for(size_t i = 0; i < indices.size(); i++)
			{
				Uint vi = m_v.insert( transform(vb->getCoord( indices[i] ), matstack.getTop()) );
				Uint ni = m_n.insert( vb->getNormal( indices[i] ) );
				Uint ti = m_t.insert( vb->getTexCoord( indices[i] ) );
				m_f[m_curmat].push_back( fi(vi+1, ni+1, ti+1) );
			}
			break;
		}
	}
	virtual void visit(GroupNode& node)
	{
		matstack.push();
		matstack.mult( node.getTransform() );

		const SceneNodeVector& nodes =  node.getChildNodes();
		for(SceneNodeVector::const_iterator it = nodes.begin(); it != nodes.end(); it++)
			(*it)->accept(*this);

		matstack.pop();
	}
	virtual void visit(ShapeNode& node)
	{
		for(GeometryIterator it = node.GeometryBegin(); it != node.GeometryEnd(); ++it)
		{
			m_curmat = m_mat.insert( it.getMaterial()->getDiffuse() );
			it.getGeometry()->accept(*this);
		}
	}
	bool write(const std::filesystem::path& sOBJfile, progress_callback progress)
	{
		float iCount = 0;
		float nCount = (float)(m_v.size() + m_n.size() + m_t.size() + m_mat.size());

		for(auto it = m_f.cbegin(); it != m_f.cend(); it++)
			nCount += it->second.size();

		for(auto it = m_l.cbegin(); it != m_l.cend(); it++)
			nCount += it->second.size();

		std::filesystem::path sMTLfile(sOBJfile);
		sMTLfile += L".mtl";

		std::ofstream obj( sOBJfile.string().c_str(), std::ios::out);

		if(!obj.is_open())
			return false;

		obj << "# OBJ " << std::endl;
		obj << "mtllib " << sMTLfile.c_str() << std::endl;

		for(Uint i = 0; i < m_v.size(); i++)
		{
			obj << "v " << m_v[i].x << " " << m_v[i].y << " " << m_v[i].z << " " << std::endl;
			progress(0.5f+((++iCount/nCount)*0.5f));
		}
		for(Uint i = 0; i < m_n.size(); i++)
		{
			obj << "vn " << m_n[i].x << " " << m_n[i].y << " " << m_n[i].z << " " << std::endl;
			progress(0.5f+((++iCount/nCount)*0.5f));
		}
		if(m_t.size() > 1)
		{
			for(Uint i = 0; i < m_t.size(); i++)
				obj << "vt " << m_t[i].x << " " << m_t[i].y << " " << std::endl;
			progress(0.5f+((++iCount/nCount)*0.5f));
		}

		for(auto it = m_f.cbegin(); it != m_f.cend(); it++)
		{
			obj << "usemtl " << it->first << std::endl;
			obj << "#RGBA " << m_mat[it->first].r << " " << m_mat[it->first].g << " " << m_mat[it->first].b << " " << 1.f - m_mat[it->first].a << std::endl;

			for(size_t i = 0; i < it->second.size(); i+=3)
			{
				obj << "f ";
				if(m_t.size() > 1)
				{
					obj << it->second[i].v << "/" << it->second[i].t << "/" << it->second[i].n << " ";
					obj << it->second[i+1].v << "/" << it->second[i+1].t << "/" << it->second[i+1].n << " ";
					obj << it->second[i+2].v << "/" << it->second[i+2].t << "/" << it->second[i+2].n << " ";
				}
				else
				{
					obj << it->second[i].v << "//"  << it->second[i].n << " ";
					obj << it->second[i+1].v << "//" << it->second[i+1].n << " ";
					obj << it->second[i+2].v << "//" << it->second[i+2].n << " ";
				}
				obj << std::endl;

				progress(0.5f+((++iCount/nCount)*0.5f));
				progress(0.5f+((++iCount/nCount)*0.5f));
				progress(0.5f+((++iCount/nCount)*0.5f));
			}
		}
		for(auto it = m_l.cbegin(); it != m_l.cend(); it++)
		{
			for(size_t i = 0; i < it->second.size(); i+=2)
			{
				obj << "l ";
				obj << it->second[i] << "// ";
				obj << it->second[i+1] << "// ";
				obj << std::endl;

				progress(0.5f+((++iCount/nCount)*0.5f));
				progress(0.5f+((++iCount/nCount)*0.5f));
			}
		}

		obj.close();

		std::ofstream mtl( sMTLfile.string().c_str(), std::ios::out);

		if(!mtl.is_open())
			return false;

		for(Uint i = 0; i < m_mat.size(); i++)
		{
			mtl << "newmtl" << i << std::endl;
			//Ka 0.921569 0.921569 0.921569
			const RGBA& d = m_mat[i];
			mtl << "Kd " << d.r << " " << d.g << " " << d.b << std::endl;
			mtl << "d " << 1.f - d.a << std::endl;
			//Ks 1.000000 1.000000 1.000000
			//illum 2
			//Ns 55.715232
			//d 1.000000
			//map_Kd
			//map_bump
			//map_d
			//refl
			//map_KS

			progress(0.5f+((++iCount/nCount)*0.5f));
		}

		mtl.close();

		return true;

	}
};

bool loadOBJfromStream(std::istream& istream, SceneNodeVector& nodes, progress_callback progress)
{
	OBJLoader loader;
	return loader.read(istream, nodes, progress);
}

class OBJPlugIn: public SceneIO::IPlugIn
{
public:
	OBJPlugIn()
	{

	}
	virtual std::wstring about() const
	{
		return L"internal OBJ Loader/Writer";
	}
	virtual Uint file_type_count() const
	{
			return 1;
	}
	virtual std::wstring file_type(Uint i) const
	{
		return L"Wavefront OBJ";
	}
	virtual std::wstring file_exts(Uint i) const
	{
		return L"*.obj";
	}
	virtual std::wstring rpath() const
	{
		return L"";
	}
	virtual bool canWrite(Uint i) const
	{
		return true;
	}
	virtual bool canRead(Uint i) const
	{
		return true;
	}

	virtual bool read(const std::wstring& sFile, Ptr<Scene> pScene, SceneIO::progress_callback& progress)
	{
		SceneNodeVector nodes;

		bool ret = OBJLoader().read(sFile, nodes, progress);

		for(eh::SceneNodeVector::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
			pScene->insertNode(*it);

		return ret;

	}
	virtual bool write(const std::wstring& sFile, Ptr<Scene> pScene, SceneIO::progress_callback& progress)
	{
		OBJExport visitor;

		float i = 0;
		float n = (float)pScene->getNodes().size();
		for(SceneNodeVector::const_iterator it = pScene->getNodes().begin();
				it != pScene->getNodes().end(); it++)
		{
			(*it)->accept( visitor );

			progress((++i/n)*0.5f);
		}

		return visitor.write(sFile, progress);

	}
};

extern "C"
{
#if defined(_MSC_VER)
__declspec(dllexport)
#endif
SceneIO::IPlugIn* XcreatePlugIn()
{
	return new OBJPlugIn();
}

SceneIO::IPlugIn* XcreateOBJPlugIn()
{
	return new OBJPlugIn();
}

}

} //end namespace

