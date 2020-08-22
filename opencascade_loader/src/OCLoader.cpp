/****************************************************************************
* Copyright (C) 2007-2010 by E.Heidt  http://teapot-viewer.sourceforge.net/ *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
****************************************************************************/

#include <SceneIO.h>
using namespace eh;

//#include <shlobj.h>
#include <set>

namespace eh
{
	namespace boost
	{
        template<typename T, typename U>
        using unordered_map = std::map<T, U>;

        template<typename T>
        using unordered_set = std::set<T>;
    }
}

#include <Precision.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <IGESCAFControl_Reader.hxx>		//IGES
#include <BRepTools.hxx>					//BREP

#include <XCAFApp_Application.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

#include <BRepPrimAPI_MakeBox.hxx>
#include <TDocStd_Document.hxx>
#include <TDF_LabelSequence.hxx>
#include <XCAFDoc_ColorType.hxx>
#include <Quantity_Color.hxx>
#include <TDF_Label.hxx>
#include <TDataStd_Name.hxx>
#include <TDF_ChildIterator.hxx>
#include <BRep_Builder.hxx>
#include <XSControl_WorkSession.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <Transfer_TransientProcess.hxx>
#include <Poly_Triangulation.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
//#include <BRepMesh_FastDiscret.hxx>
#include <TopExp_Explorer.hxx>
#include <BRep_Tool.hxx>
#include <TShort_Array1OfShortReal.hxx>
#include <Message_ProgressIndicator.hxx>
#include <TopoDS_Edge.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <TColStd_HArray1OfReal.hxx>
#include <BRep_Builder.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_TangentialDeflection.hxx>

#include <BOPTools_AlgoTools3D.hxx>
#include <TColgp_Array1OfPnt2d.hxx>

#include <BRepGProp_Face.hxx>

#if defined(_MSC_VER)

#pragma comment ( lib, "TKBO" )		//BOPTools

//#pragma comment ( lib, "TKStep" )	//DataExchange
#pragma comment ( lib, "TKXSBase" )	//DataExchange
#pragma comment ( lib, "TKXCAF" )	//DataExchange

#pragma comment ( lib, "TKXDESTEP" )	//DataExchange
#pragma comment ( lib, "TKXDEIGES" )	//DataExchange

#pragma comment ( lib, "TKTopAlgo" )
#pragma comment ( lib, "TKPrim" )
#pragma comment ( lib, "TKLCAF" )
#pragma comment ( lib, "TKernel" )
#pragma comment ( lib, "TKBREP" )
#pragma comment ( lib, "TKMath" )
#pragma comment ( lib, "TKMesh" )
#pragma comment ( lib, "TKG3d" )
#pragma comment ( lib, "TKGeomBase" )
#pragma comment ( lib, "TKG2D" )

#endif

class OCLoader: public SceneIO::IPlugIn
{
private:
	Ptr<IVertexBuffer> m_pVB;

	bool m_bCount;
	float m_nCount;
	float m_iCount;

	Ptr<Scene> m_pScene;
	SceneIO::progress_callback progress;

	static const int HashUpper = INT_MAX;
	const double m_fDeflection;
	const double m_fDeflAngle;

	boost::unordered_set<int> m_reffered_shapes;
	boost::unordered_map<int, Ptr<SceneNode> > m_hashes;

	Ptr<SceneNode> createShape(const TDF_Label& label);
	Ptr<SceneNode> createShape(const TopoDS_Shape& aShape, const Handle_XCAFDoc_ColorTool& Colors);
	bool makeFace(const TopoDS_Face& aFace, Uint_vec& indices);
	bool makeEdge(const TopoDS_Edge& aEdge, Uint_vec& indices);

	void iterateChilds(const TDF_Label& label, Matrix tra = Matrix(), bool bRef = false);
	void proceedChilds(const TDF_Label& label);

public:
	OCLoader():
		m_pVB(nullptr),
		m_bCount(false),
		m_nCount(0),
		m_iCount(0),
		m_pScene(nullptr),
		m_fDeflection(1),
		m_fDeflAngle(0.5)
	{
	}

	virtual std::wstring about() const
	{
		return L"OpenCascade_Loader";
	}

	virtual Uint file_type_count() const
	{
		return 3;
	}

	virtual std::wstring file_type(Uint i) const
	{
		switch(i)
		{
		case 0: return L"STEP Models";
		case 1: return L"IGES Models";
		case 2: return L"BREP Models";
		default: return L"";
		}
	}
	virtual std::wstring file_exts(Uint i) const
	{
		switch(i)
		{
		case 0: return L"*.stp;*.step";
		case 1: return L"*.igs;*.iges";
		case 2: return L"*.brep";
		default: return L"";
		}
	}
	virtual std::wstring rpath() const
	{
			return L"";
	}
	virtual bool canWrite(Uint i) const
	{
		return false;
	}
	virtual bool canRead(Uint i) const
	{
		return true;
	}

	virtual bool read(const std::wstring& sFile, Ptr<Scene> pScene, SceneIO::progress_callback& progress);
	virtual bool write(const std::wstring& sFile, Ptr<Scene> pScene, SceneIO::progress_callback& progress)
	{
		return false;
	}

};

bool OCLoader::read(const std::wstring& sFile, Ptr<Scene> pScene, SceneIO::progress_callback& progress_callback)
{
	m_iCount = m_nCount = 0;
	m_bCount = false;
	progress = progress_callback;
	m_pScene = pScene;

	m_pVB = CreateVertexBuffer( sizeof(Vec3)*2 );
	m_reffered_shapes.clear();
	m_hashes.clear();

	class MyProgressIndicator : public Message_ProgressIndicator
	{
		SceneIO::progress_callback& progress;
	public:
		MyProgressIndicator(SceneIO::progress_callback& cb):progress(cb)
		{
		}
		virtual Standard_Boolean Show( const Standard_Boolean force )
		{
			progress((float)this->GetPosition());
			return true;
		}
	};

	Handle_Message_ProgressIndicator anIndicator = new MyProgressIndicator(progress);
	anIndicator->NewScope (50, "Reading");

	std::wstring sExt = SceneIO::File( sFile ).getExtension();

	if( boost::iequals( sExt, L".stp" ) || boost::iequals( sExt, L".step" ) )
	{
		Handle_TDocStd_Document pDoc = new TDocStd_Document("STEP");

		STEPCAFControl_Reader aReader;
		aReader.SetColorMode(true);
		aReader.SetNameMode(true);
		aReader.SetLayerMode(true);

		IFSelect_ReturnStatus status = aReader.ReadFile( std::string(sFile.begin(), sFile.end()).c_str() );

		if (status != IFSelect_RetDone)
			return false;

		aReader.Reader().WS()->MapReader()->SetProgress( anIndicator);

		Standard_Boolean ret = aReader.Transfer( pDoc);

		if(ret == Standard_False)
			return false;

		proceedChilds(pDoc->Main());

		return true;
	}
	else if(  boost::iequals( sExt, L".igs" ) || boost::iequals( sExt, L".iges" ))
	{
		Handle_TDocStd_Document pDoc = new TDocStd_Document("IGES");

		IGESCAFControl_Reader aReader;
		aReader.SetColorMode(true);
		aReader.SetNameMode(true);
		aReader.SetLayerMode(true);

		IFSelect_ReturnStatus status = aReader.ReadFile( std::string(sFile.begin(), sFile.end()).c_str() );

		if (status != IFSelect_RetDone)
			return false;

		aReader.WS()->MapReader()->SetProgress( anIndicator);

		Standard_Boolean ret = aReader.Transfer( pDoc);

		if(ret == Standard_False)
			return false;

		proceedChilds(pDoc->Main());

		return true;
	}
	else if( boost::iequals( sExt, L".brep" ) )
	{
		TopoDS_Shape aShape;
		BRep_Builder aBuilder;
		Standard_Boolean result = BRepTools::Read(aShape,
							  std::string(sFile.begin(), sFile.end()).c_str(),
							  aBuilder, anIndicator);

        if(result)
        {
            m_bCount = true;
            m_nCount = 0;
            m_iCount = 0;
            createShape(aShape, NULL);

            m_bCount = false;

            pScene->insertNode( createShape(aShape, NULL) );

            return true;
        }
	}

	return false;
}

void OCLoader::proceedChilds(const TDF_Label& label)
{
	m_bCount = true;
	m_nCount = 0;
	m_iCount = 0;

	iterateChilds(label);

	m_bCount = false;
	iterateChilds(label);
}

void OCLoader::iterateChilds(const TDF_Label& label, Matrix tra /*= Matrix()*/, bool bRef /*= false*/)
{
	Handle(XCAFDoc_ColorTool) aColorTool = XCAFDoc_DocumentTool::ColorTool(label);
	Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(label);

	int hash = 0;
	TopoDS_Shape aShape;
	if(aShapeTool->GetShape(label,aShape))
	{
		hash = aShape.HashCode(HashUpper);

		const TopLoc_Location& aLoc = aShape.Location();
		gp_Trsf trs = aLoc.Transformation();

		Matrix m;

		m[0] = (float)trs.Value(1,1);	m[4] = (float)trs.Value(1,2);	m[8]  = (float)trs.Value(1,3);	m[12] = (float)trs.Value(1,4);
		m[1] = (float)trs.Value(2,1);	m[5] = (float)trs.Value(2,2);	m[9]  = (float)trs.Value(2,3);	m[13] = (float)trs.Value(2,4);
		m[2] = (float)trs.Value(3,1);	m[6] = (float)trs.Value(3,2);	m[10] = (float)trs.Value(3,3);	m[14] = (float)trs.Value(3,4);
		m[3] = 0;			m[7] = 0;			m[11] = 0;			m[15] = 1;

		tra = m * tra;
	}

	TCollection_ExtendedString name;
	Handle(TDataStd_Name) N;
	bool bName = false;
	if ( label.FindAttribute(TDataStd_Name::GetID(),N))
	{
		name = N->Get();
		bName = true;
	}

	//TRACE(_T("\n%sL%X %s TL=%d A=%d S=%d Cd=%d SS=%d F=%d R=%d CT=%d Sub=%d"),
	//	tab,
	//	hash,
	//	name.ToExtString(),
	//	aShapeTool->IsTopLevel(label),
	//	aShapeTool->IsAssembly(label),
	//	aShapeTool->IsShape(label),
	//	aShapeTool->IsCompound(label),
	//	aShapeTool->IsSimpleShape(label),
	//	aShapeTool->IsFree(label),
	//	aShapeTool->IsReference(label),
	//	aShapeTool->IsComponent(label),
	//	aShapeTool->IsSubShape(label)
	//	);

	TDF_Label ref;
	if(aShapeTool->IsReference(label) && aShapeTool->GetReferredShape(label, ref))
	{
		iterateChilds(ref, tra, true);
	}

	if(bRef || m_reffered_shapes.find(hash) == m_reffered_shapes.end())
	{
		TopoDS_Shape aShape;
		if(bRef && aShapeTool->GetShape(label, aShape))
			m_reffered_shapes.insert(aShape.HashCode(HashUpper));

		if( aShapeTool->IsSimpleShape(label) && (bRef || aShapeTool->IsFree(label)))
		{
			if(Ptr<SceneNode> pShape = createShape( label ))
			{
				m_pScene->insertNode( GroupNode::create( pShape, tra ) );
			}
		}
		else
		{
			for(TDF_ChildIterator it(label); it.More(); it.Next())
			{
				iterateChilds(it.Value(), tra, bRef);
			}
		}
	}
}

Ptr<SceneNode> OCLoader::createShape(const TDF_Label& label)
{
	Handle_XCAFDoc_ShapeTool aAssembly = XCAFDoc_DocumentTool::ShapeTool(label);
	const TopoDS_Shape& aShape = aAssembly->GetShape(label);
	Handle_XCAFDoc_ColorTool Colors = XCAFDoc_DocumentTool::ColorTool(label);
	return createShape( aShape, Colors );
}
Ptr<SceneNode> OCLoader::createShape(const TopoDS_Shape& aShape, const Handle_XCAFDoc_ColorTool& Colors)
{
	int hash = aShape.HashCode(HashUpper);

	if(m_hashes.find(hash) != m_hashes.end())
		return m_hashes[hash];

	RGBA color = RGBA(0.7f,0.7f,0.7f);

	Quantity_Color aColor;
	if( !Colors.IsNull() && Colors->GetColor( aShape, XCAFDoc_ColorSurf, aColor) )
	{
		color = RGBA((Float)aColor.Red(), (Float)aColor.Green(), (Float)aColor.Blue());
	}


	Ptr<ShapeNode> shape = ShapeNode::create();

	boost::unordered_map< RGBA, Uint_vec > faces;
	Uint_vec edges;

	for (TopExp_Explorer anFaceExp(aShape, TopAbs_FACE); anFaceExp.More(); anFaceExp.Next())
	{
		const TopoDS_Shape& bShape = anFaceExp.Current();
		const TopoDS_Face& aFace = TopoDS::Face(bShape);

		if(!Colors.IsNull() && Colors->GetColor( aFace, XCAFDoc_ColorSurf, aColor) )
			color = RGBA((Float)aColor.Red(), (Float)aColor.Green(), (Float)aColor.Blue());

		if (aFace.IsNull() == Standard_False)
			 makeFace(aFace, faces[color] );
	}

	for( boost::unordered_map< RGBA, Uint_vec >::const_iterator it = faces.begin(); it != faces.end(); it++)
	{
		if(it->second.size() > 0)
			shape->addGeometry( Material::create( it->first ), Geometry::create(Geometry::TRIANGLES, m_pVB, it->second) );
	}

	for (TopExp_Explorer anEdgeExp (aShape, TopAbs_EDGE); anEdgeExp.More(); anEdgeExp.Next())
	{
		const TopoDS_Shape& aShape = anEdgeExp.Current();

		const TopoDS_Edge& aEdge = TopoDS::Edge (aShape);

		if (aEdge.IsNull() == Standard_False)
			makeEdge(aEdge, edges);
	}

	if(edges.size() > 0)
		shape->addGeometry( Material::Black(), Geometry::create(Geometry::LINES, m_pVB, edges) );

	if(m_bCount)
		return nullptr;
	else
		return m_hashes[hash] = shape;
}

template<class T>
Vec3 to_Vector3D(const T& t)
{
	return Vec3((Float)t.X(), (Float)t.Y(), (Float)t.Z());
}

bool OCLoader::makeFace(const TopoDS_Face& aFace, Uint_vec& indices)
{
	if(m_bCount)
	{
		m_nCount += 1;
		return NULL;
	}
	else
	{
		m_iCount += 1;
		progress(0.5f+(m_iCount/m_nCount)/2);
	}

	TopLoc_Location aLoc;
	Handle(Poly_Triangulation) aTri = BRep_Tool::Triangulation (aFace, aLoc);

	if( aTri.IsNull() || aTri->Deflection() > m_fDeflection+ Precision::Confusion() )
	{
		// Triangulate the face by the standard OCC mesher
		BRepMesh_IncrementalMesh IM(aFace, m_fDeflection, Standard_False, m_fDeflAngle);
		//BRepMesh_FastDiscret DM(m_fDeflection, aFace, Bnd_Box(), Standard_True, m_fDeflAngle);
	}

	BRepAdaptor_Surface sf(aFace, Standard_False);
	BRepGProp_Face prop(aFace);

	aTri = BRep_Tool::Triangulation(aFace, aLoc);

	if (aTri.IsNull() == Standard_False)
	{
//		const Standard_Integer nNodes(aTri->NbNodes());
		const Standard_Integer nTriangles(aTri->NbTriangles());
		const TColgp_Array1OfPnt&    arrPolyNodes = aTri->Nodes();
		const Poly_Array1OfTriangle& arrTriangles = aTri->Triangles();
		const TColgp_Array1OfPnt2d&  arrUvnodes = aTri->UVNodes();

		Handle(Geom_Surface) aSurface = BRep_Tool::Surface(aFace, aLoc);

		TopAbs_Orientation orientation = aFace.Orientation();

		for (Uint i = 0; i < (Uint)nTriangles; i++)
		{
			int ia = 0, ib = 0, ic = 0;
			const Poly_Triangle& tri = arrTriangles(i+1);

			if(orientation == TopAbs_REVERSED)
				tri.Get(ia,ic,ib);
			else
				tri.Get(ia,ib,ic);

			gp_Pnt va, vb, vc;
			va = arrPolyNodes(ia);
			va.Transform(aLoc.Transformation());
			vb = arrPolyNodes(ib);
			vb.Transform(aLoc.Transformation());
			vc = arrPolyNodes(ic);
			vc.Transform(aLoc.Transformation());

			gp_Dir na, nb, nc;
			BOPTools_AlgoTools3D::GetNormalToSurface( aSurface, arrUvnodes(ia).X(), arrUvnodes(ia).Y(), na);
			na.Transform(aLoc.Transformation());
			BOPTools_AlgoTools3D::GetNormalToSurface( aSurface, arrUvnodes(ia).X(), arrUvnodes(ia).Y(), nb);
			nb.Transform(aLoc.Transformation());
			BOPTools_AlgoTools3D::GetNormalToSurface( aSurface, arrUvnodes(ia).X(), arrUvnodes(ia).Y(), nc);
			nc.Transform(aLoc.Transformation());

			//if(orientation == TopAbs_REVERSED)
			//	normals[(Uint)ia-1] = to_Vector3D(aD).getNormalized() * -1;
			//else
			//	normals[(Uint)ia-1] = to_Vector3D(aD).getNormalized();

			indices.push_back( m_pVB->addVertex( to_Vector3D(va), to_Vector3D(na), Vec3::Null()) );
			indices.push_back( m_pVB->addVertex( to_Vector3D(vb), to_Vector3D(nb), Vec3::Null()) );
			indices.push_back( m_pVB->addVertex( to_Vector3D(vc), to_Vector3D(nc), Vec3::Null()) );
		}

		return true;
	}

	return false;


}
bool OCLoader::makeEdge(const TopoDS_Edge& aEdge, Uint_vec& indices)
{
	if(m_bCount)
	{
		m_nCount += 1;
		return false;
	}
	else
	{
		m_iCount += 1;
		progress(0.5f+(m_iCount/m_nCount)/2);
	}

	TopLoc_Location aLoc;
	Handle(Poly_Polygon3D) aPol = BRep_Tool::Polygon3D (aEdge, aLoc);
	Standard_Boolean isTessellate (Standard_False);

	if (aPol.IsNull())
		isTessellate = Standard_True;
	// Check the existing deflection
	else if (aPol->Deflection() > m_fDeflection + Precision::Confusion() && BRep_Tool::IsGeometric(aEdge))
		isTessellate = Standard_True;

	if (isTessellate && BRep_Tool::IsGeometric(aEdge))
	{
		//try to find PolygonOnTriangulation
		Handle(Poly_PolygonOnTriangulation) aPT;
		Handle(Poly_Triangulation) aT;
		TopLoc_Location aL;

		Standard_Boolean found = Standard_False;
		for(int i = 1; Standard_True; i++)
		{
			BRep_Tool::PolygonOnTriangulation(aEdge, aPT, aT, aL, i);

			if(aPT.IsNull() || aT.IsNull()) break;

			if(aPT->Deflection() <= m_fDeflection + Precision::Confusion() && aPT->HasParameters())
			{
				found = Standard_True;
				break;
			}
		}

		if(found)
		{

			BRepAdaptor_Curve aCurve(aEdge);
			Handle(TColStd_HArray1OfReal) aPrs = aPT->Parameters();
			Standard_Integer nbNodes = aPT->NbNodes();
			TColgp_Array1OfPnt arrNodes(1, nbNodes);
			TColStd_Array1OfReal arrUVNodes(1, nbNodes);

			for(int i = 1; i <= nbNodes; i++)
			{
				arrUVNodes(i) = aPrs->Value(aPrs->Lower() + i - 1);
				arrNodes(i) = aCurve.Value(arrUVNodes(i));
			}
			aPol = new Poly_Polygon3D(arrNodes, arrUVNodes);
			aPol->Deflection (aPT->Deflection());
		}
		else
		{

			BRepAdaptor_Curve aCurve(aEdge);
			const Standard_Real aFirst = aCurve.FirstParameter();
			const Standard_Real aLast  = aCurve.LastParameter();

			GCPnts_TangentialDeflection TD (aCurve, aFirst, aLast, m_fDeflAngle, m_fDeflection, 2);
			const Standard_Integer nbNodes = TD.NbPoints();

			TColgp_Array1OfPnt arrNodes(1, nbNodes);
			TColStd_Array1OfReal arrUVNodes(1, nbNodes);
			for (int i = 1; i <= nbNodes; i++)
			{
				arrNodes(i) = TD.Value(i);
				arrUVNodes(i) = TD.Parameter(i);
			}
			aPol = new Poly_Polygon3D(arrNodes, arrUVNodes);
			aPol->Deflection (m_fDeflection);
		}

		BRep_Builder aBld;
		aBld.UpdateEdge (aEdge, aPol);
	}


	const Standard_Integer nNodes (aPol->NbNodes());
	const TColgp_Array1OfPnt& arrPolyNodes = aPol->Nodes();

	for(int i = 0; i < nNodes-1; i++)
	{
		gp_Pnt p1 = arrPolyNodes(i+1);
		p1.Transform(aLoc.Transformation());

		gp_Pnt p2 = arrPolyNodes(i+2);
		p2.Transform(aLoc.Transformation());

		indices.push_back( m_pVB->addVertex( to_Vector3D(p1), Vec3::Null(), Vec3::Null())  );
		indices.push_back( m_pVB->addVertex( to_Vector3D(p2), Vec3::Null(), Vec3::Null())  );
	}

	return true;


}

extern "C"
{
#if defined(_MSC_VER)
__declspec(dllexport)
#endif
SceneIO::IPlugIn* XcreatePlugIn()
{
	return new OCLoader();
}
}
