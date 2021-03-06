
#include "stdafx.h"
#include "pathfinder.h"


using namespace ai;


cPathFinder::cPathFinder()
{
}

cPathFinder::~cPathFinder()
{
	Clear();
}


bool cPathFinder::Create(const int vertexCount)
{
	m_vertices.reserve(vertexCount);
	m_areas.reserve(32);
	return true;
}


bool cPathFinder::Read(const StrPath &fileName)
{
	Clear();

	std::ifstream ifs(fileName.c_str());
	RETV(!ifs.is_open(), false);

	int state = 0;
	ai::sVertex vtx;

	char line[256];
	while (ifs.getline(line, sizeof(line)))
	{
		std::stringstream ss(line);
		
		char token[64];
		switch (state)
		{
		case 0:
		{
			ss >> token;
			if (!strcmp(token, "Vertex"))
				state = 1;
		}
		break;

		case 1:
		{
			ss >> token;
			assert(!strcmp(token, "type"));
			ss >> vtx.type;
			state = 2;
		}
		break;

		case 2:
		{
			ss >> token;
			assert(!strcmp(token, "pos"));
			ss >> vtx.pos.x >> vtx.pos.y >> vtx.pos.z;
			state = 3;
		}
		break;

		case 3:
		{
			ss >> token;
			assert(!strcmp(token, "dir"));
			//ss >> vtx.dir.x >> vtx.dir.y >> vtx.dir.z;
			state = 4;
		}
		break;

		case 4:
		{
			ss >> token;
			assert(!strcmp(token, "edge"));
			int idx = 0;
			int cnt = 0;
			do {
				ss >> vtx.edge[idx++];
				++cnt;
			} while (!ss.eof() && (cnt < ai::sVertex::MAX_EDGE));

			AddVertex(vtx);
			
			state = 0;
			for (int i=0; i < ai::sVertex::MAX_EDGE; ++i)
				vtx.edge[i] = -1;
		}
		break;
		}
	}

	return true;
}


// Write Format
// Vertex
// type 1
//	pos 1 1 1
//	dir 0 0 1
//	edge 0 1 2
bool cPathFinder::Write(const StrPath &fileName)
{
	using namespace std;

	ofstream ofs(fileName.c_str());
	RETV(!ofs.is_open(), false);

	for (auto &v : m_vertices)
	{
		ofs << "Vertex" << endl;
		ofs << "\ttype " << v.type << endl;
		ofs << "\tpos " << v.pos.x << " " << v.pos.y << " " << v.pos.z << endl;
		//ofs << "\tdir " << v.dir.x << " " << v.dir.y << " " << v.dir.z << endl;
		ofs << "\tdir " << 0 << " " << 0 << " " << 0 << endl;
		ofs << "\tedge ";
		for (int i = 0; i < sVertex::MAX_EDGE; ++i)
		{
			if (v.edge[i] < 0)
				break;
			ofs << v.edge[i] << " ";
		}
		ofs << endl;
	}

	return true;
}


bool cPathFinder::AddVertex(const sVertex &vtx)
{
	m_vertices.push_back(vtx);
	return true;
}


bool cPathFinder::AddEdge(const int vtxIdx, const int addEdgeIdx)
{
	RETV(vtxIdx < 0, false);
	RETV((int)m_vertices.size() <= vtxIdx, false);
	RETV(vtxIdx == addEdgeIdx, false);

	ai::sVertex &vtx = m_vertices[vtxIdx];

	bool isAlreadyExist = false;
	for (int i = 0; i < ai::sVertex::MAX_EDGE; ++i)
	{
		if (0 > vtx.edge[i])
			break;

		if (addEdgeIdx == vtx.edge[i])
		{
			isAlreadyExist = true;
			break;
		}
	}

	RETV(isAlreadyExist, false);

	// push back
	for (int i = 0; i < ai::sVertex::MAX_EDGE; ++i)
	{
		if (0 > vtx.edge[i])
		{
			vtx.edge[i] = addEdgeIdx;
			return true;			
		}
	}

	return false;
}



// remove edge vtxIdx <-> removeEdgeIdx
bool cPathFinder::RemoveEdgeEachOther(const int vtxIdx, const int removeEdgeIdx)
{
	RemoveEdge(vtxIdx, removeEdgeIdx);
	RemoveEdge(removeEdgeIdx, vtxIdx);
	return true;
}


// remove edge vtxIdx -> removeEdgeIdx
bool cPathFinder::RemoveEdge(const int vtxIdx, const int removeEdgeIdx)
{
	RETV(vtxIdx < 0, false);
	RETV((int)m_vertices.size() <= vtxIdx, false);

	ai::sVertex &vtx = m_vertices[vtxIdx];

	for (int i = 0; i < ai::sVertex::MAX_EDGE; ++i)
	{
		if (0 > vtx.edge[i])
			break;

		if (removeEdgeIdx == vtx.edge[i])
		{
			for (int k = i; k < ai::sVertex::MAX_EDGE - 1; ++k)
				vtx.edge[k] = vtx.edge[k + 1];
			vtx.edge[ai::sVertex::MAX_EDGE - 1] = -1;
			return true;
		}
	}

	return false;
}


bool cPathFinder::RemoveVertex(const int index)
{
	// remove edge and decrease index if greater than remove index
	for (auto &v : m_vertices)
	{
		for (int i = 0; i < sVertex::MAX_EDGE; ++i)
		{
			if (0 > v.edge[i])
				break;

			if (index == v.edge[i]) // rotate left
			{
				for (int k = i; k < ai::sVertex::MAX_EDGE - 1; ++k)
					v.edge[k] = v.edge[k + 1];
				v.edge[ai::sVertex::MAX_EDGE - 1] = -1;

				--i; // for loop bugfix
			}
			else if (index < v.edge[i])
			{
				--v.edge[i]; // decrease index
			}
		}
	}

	// remove index vertex
	common::rotatepopvector(m_vertices, index);
	
	return true;
}


int cPathFinder::GetNearestVertex(const Vector3 &pos) const
{
	RETV(m_vertices.empty(), -1);
	
	int cnt = 0;
	int indices[128];

	if (m_areas.empty())
	{
		for (u_int i = 0; i < m_vertices.size(); ++i)
			if (cnt < ARRAYSIZE(indices))
				indices[cnt++] = (int)i;
	}
	else
	{
		for (auto &area : m_areas)
		{
			if (area.rect.IsIn(pos.x, pos.z))
			{
				for (auto idx : area.indices)
					if (cnt < ARRAYSIZE(indices))
						indices[cnt++] = idx;
				break;
			}
		}
	}

	int idx = 0;
	float minLen = FLT_MAX;

	for (int i=0; i < cnt; ++i)
	{
		auto &vtx = m_vertices[ indices[i]];
		const float len = vtx.pos.LengthRoughly(pos);
		if (len < minLen)
		{
			idx = indices[i];
			minLen = len;
		}
	}

	return idx;
}


// return = minimum( distance(pos, nearest vertex) + distance(nearest vertex, end) )
int cPathFinder::GetNearestVertex(const Vector3 &pos, const Vector3 &end) const
{
	RETV(m_vertices.empty(), -1);

	int cnt = 0;
	int indices[128];

	if (m_areas.empty())
	{
		for (u_int i = 0; i < m_vertices.size(); ++i)
			if (cnt < ARRAYSIZE(indices))
				indices[cnt++] = (int)i;
	}
	else
	{
		for (auto &area : m_areas)
		{
			if (area.rect.IsIn(pos.x, pos.z))
			{
				for (auto idx : area.indices)
					if (cnt < ARRAYSIZE(indices))
						indices[cnt++] = idx;
				break;
			}
		}
	}

	int idx = 0;
	float minLen = FLT_MAX;

	for (int i = 0; i < cnt; ++i)
	{
		auto &vtx = m_vertices[indices[i]];
		const float len = vtx.pos.LengthRoughly(pos) + vtx.pos.LengthRoughly(end);		
		if (len < minLen)
		{
			idx = indices[i];
			minLen = len;
		}
	}

	return idx;
}


int cPathFinder::GetNearestArea(const Vector3 &pos) const
{
	for (u_int i=0; i < m_areas.size(); ++i)
	{
		auto &area = m_areas[i];
		if (area.rect.IsIn(pos.x, pos.z))
			return i;
	}
	return -1;
}


bool cPathFinder::Find(const Vector3 &start, const Vector3 &end,
	OUT vector<Vector3> &out
)
{
	const int startIdx = m_areas.empty()? GetNearestVertex(start) : GetNearestVertex(start, end);
	if (startIdx < 0)
		return false;

	const int endIdx = m_areas.empty() ? GetNearestVertex(end) : GetNearestVertex(end, start);
	if (endIdx < 0)
		return false;

	ZeroMemory(m_edges_visit, sizeof(m_edges_visit));
	ZeroMemory(m_edges_len, sizeof(m_edges_len));

	vector<int> candidate;
	candidate.reserve(m_vertices.size());
	candidate.push_back(startIdx);
	m_vertices[startIdx].startLen = 0;
	m_vertices[startIdx].endLen = Distance(start, end);

	int loopCount1 = 0; // debug, loop count
	int loopCount2 = 0; // debug, insertion count
	bool isFind = false;
	while (!candidate.empty())
	{
		const int curIdx = candidate.front();
		rotatepopvector(candidate, 0);

		sVertex &curVtx = m_vertices[curIdx];

		if (endIdx == curIdx)
		{
			isFind = true;
			break;
		}

		for (int i = 0; i < sVertex::MAX_EDGE; ++i)
		{
			if (curVtx.edge[i] < 0)
				break;

			const int nextIdx = curVtx.edge[i];
			sVertex &nextVtx = m_vertices[nextIdx];

			if (m_edges_visit[curIdx][nextIdx])
				continue;

			nextVtx.startLen = curVtx.startLen + Distance(curVtx.pos, nextVtx.pos) + 0.00001f;
			nextVtx.endLen = Distance(end, nextVtx.pos);
			m_edges_visit[curIdx][nextIdx] = true;
			m_edges_visit[nextIdx][curIdx] = true;
			m_edges_len[curIdx][nextIdx] = nextVtx.startLen + nextVtx.endLen;
			m_edges_len[nextIdx][curIdx] = nextVtx.startLen + nextVtx.endLen;

			// sorting candidate
			// value = minimum( startLen + endLen )
			bool isInsert = false;
			for (u_int k = 0; k < candidate.size(); ++k)
			{
				++loopCount1;

				sVertex &compVtx = m_vertices[candidate[k]];
				if ((compVtx.endLen + compVtx.startLen) 
						> (nextVtx.endLen + nextVtx.startLen))
				{
					++loopCount2;

					candidate.push_back(nextIdx);
					common::rotateright2(candidate, k);
					isInsert = true;
					break;
				}
			}

			if (!isInsert)
				candidate.push_back(nextIdx);
		}
	}

	if (!isFind)
		return false;

	// tracking end point to start point
	vector<int> verticesIndices;

	out.push_back(m_vertices[endIdx].pos);
	verticesIndices.push_back(endIdx);

	ZeroMemory(m_edges_visit, sizeof(m_edges_visit));

	int curIdx = endIdx;
	while (curIdx != startIdx)
	{
		float minEdge = FLT_MAX;
		int minIdx = -1;
		sVertex &vtx = m_vertices[curIdx];
		for (int i = 0; i < sVertex::MAX_EDGE; ++i)
		{
			if (vtx.edge[i] < 0)
				break;
			if (m_edges_visit[curIdx][vtx.edge[i]])
				continue;

			const float len = m_edges_len[curIdx][vtx.edge[i]];
			if (0 == len)
				continue;

			if (minEdge > len)
			{
				minEdge = len;
				minIdx = i;
			}
		}

		if (minIdx < 0)
			break; // error occur

		m_edges_visit[curIdx][minIdx] = true;
		m_edges_visit[minIdx][curIdx] = true;
		const int parentIdx = vtx.edge[minIdx];
		out.push_back(m_vertices[parentIdx].pos);
		verticesIndices.push_back(parentIdx);
		curIdx = parentIdx;	
	}

	std::reverse(out.begin(), out.end());
	std::reverse(verticesIndices.begin(), verticesIndices.end());

	// Optimize Start Area
	const int startAreaId = GetNearestArea(start);
	if (startAreaId >= 0)
	{
		int cnt = 0;
		bool isPathNode = false;
		for (u_int i = 0; i < out.size()-1; ++i)
		{
			auto &p = out[i];
			if (!m_areas[startAreaId].rect.IsIn(p.x, p.z))
				break;

			if (1 == m_vertices[verticesIndices[i]].type)
				isPathNode = true;

			++cnt;
		}

		if (isPathNode)
			--cnt;

		for (int i = 0; i < cnt; ++i)
		{
			common::rotatepopvector(out, 0);
			common::rotatepopvector(verticesIndices, 0);				
		}
	}

	// Optimize End Area
	const int endAreaId = GetNearestArea(end);
	if (endAreaId >= 0)
	{
		int cnt = 0;
		bool isPathNode = false;
		for (int i = (int)out.size() - 1; i >= 0; --i)
		{
			auto &p = out[i];
			if (!m_areas[endAreaId].rect.IsIn(p.x, p.z))
				break;

			if (1 == m_vertices[verticesIndices[i]].type)
				isPathNode = true;

			++cnt;
		}

		if (isPathNode)
			--cnt;

		for (int i = 0; i < cnt; ++i)
			out.pop_back();
	}

	// 같은 에어리어에서 움직일 때, path 버텍스 타입으로 인해 없어지지 않았던 버텍스를
	// 지운다. 같은 에어리어 내에서는 기준 버텍스 없이 움직일 수 있다.
	if ((startAreaId >= 0) && (startAreaId == endAreaId) && !out.empty())
		out.pop_back();

	out.push_back(end);

	return true;
}


// Manhatan Distance
float cPathFinder::Distance_Manhatan(const Vector3 &p0, const Vector3 &p1) const
{
	return abs(p0.x - p1.x) + abs(p0.y - p1.y) + abs(p0.z - p1.z);
}

// Normal Distance
float cPathFinder::Distance(const Vector3 &p0, const Vector3 &p1) const
{
	return p0.Distance(p1);
}


// Add Area
bool cPathFinder::AddArea(const sRectf &area)
{
	// Check Already Exist
	for (auto &p : m_areas)
	{
		if (p.rect == area)
			return false;
	}
	
	sArea data;
	data.id = m_areas.size();
	data.rect = area;
	data.indices.reserve(16);

	// add vertex
	for (u_int i=0; i < m_vertices.size(); ++i)
	{
		sVertex &v = m_vertices[i];
		if (area.IsIn(v.pos.x, v.pos.z))
			data.indices.push_back(i);
	}

	m_areas.push_back(data);
	return true;
}


void cPathFinder::Clear()
{
	m_vertices.clear();
}
