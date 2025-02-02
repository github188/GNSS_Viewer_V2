#include "StdAfx.h"
#include "SkytraqKml.h"
#include "GPSDlg.h"
#include <algorithm>

using namespace std;
CSkyTraqKml::CSkyTraqKml(void)
{
	iniWriteKml = false;
    msg_gpgsa = msg_glgsa = msg_bdgsa = msg_gagsa = NULL;
    msg_gpgsv = msg_glgsv = msg_bdgsv = msg_gagsv = NULL;
	satellites_gp = satellites_gl = satellites_bd = satellites_ga = satellites_gi = NULL;
}

CSkyTraqKml::~CSkyTraqKml(void)
{

}

void CSkyTraqKml::Init(const char *name, int color, bool kml3d, bool bPointList, bool bNoPointText, bool bDetailInfo)
{
	kmlFile.Open(name, CFile::modeReadWrite | CFile::modeCreate);	
	lineColor = color;
	iniWriteKml = true;
	convert3d = kml3d;
	pointList = bPointList;
	noPointText = bNoPointText;
	detailInfo = bDetailInfo;

    msg_gpgsa = msg_glgsa = msg_bdgsa = msg_gagsa = NULL;
    msg_gpgsv = msg_glgsv = msg_bdgsv = msg_gagsv = NULL;
	satellites_gp = satellites_gl = satellites_bd = satellites_ga = satellites_gi = NULL;

	if(pointList)
	{
		pls.RemoveAll();
		strPointList = "<Folder id=\"MyPoints\">"\
		"<Style id=\"PointStyle\"><IconStyle><color>ffffffff</color><Icon><href>http://maps.google.com/mapfiles/kml/shapes/open-diamond.png</href></Icon></IconStyle></Style>" \
		"<Style id=\"PointStyle2\"><IconStyle><color>ffff0000</color><Icon><href>http://maps.google.com/mapfiles/kml/shapes/open-diamond.png</href></Icon></IconStyle></Style>" \
		"<Style id=\"PointStyle3\"><IconStyle><color>ff00ff00</color><Icon><href>http://maps.google.com/mapfiles/kml/shapes/open-diamond.png</href></Icon></IconStyle></Style>" \
		"<Style id=\"PointStyle4\"><IconStyle><color>ffff00ff</color><Icon><href>http://maps.google.com/mapfiles/kml/shapes/open-diamond.png</href></Icon></IconStyle></Style>";

		if(detailInfo)
		{
			strPointList += "<style type=\"text/css\">" \
			".tg  {border-collapse:collapse;border-spacing:0;}" \
			".tg td{font-family:Arial, sans-serif;font-size:14px;padding:0px 3px;border-style:solid;border-width:1px;overflow:hidden;word-break:normal;}" \
			".tg th{font-family:Arial, sans-serif;font-size:14px;font-weight:normal;padding:0px 3px;border-style:solid;border-width:1px;overflow:hidden;word-break:normal;}" \
			".tg .tg-mlgx{font-size:small;font-family:Arial, Helvetica, sans-serif !important;;color:#cb0000;text-align:center;vertical-align:top}" \
			".tg .tg-lt90{font-size:small;font-family:Arial, Helvetica, sans-serif !important;;text-align:center;vertical-align:top}" \
			".tg .tg-i8eo{font-size:small;font-family:Arial, Helvetica, sans-serif !important;;color:#3531ff;text-align:center;vertical-align:top}" \
			"</style>";
		}
		strPointList += "<name>Points</name>\r\n";
		pls.AddTail(strPointList);
		strPointList.Empty();
	}
}

void CSkyTraqKml::PushOnePoint(double lon, double lat, double alt, const CString& ts, QualityMode q)
{
	if(iniWriteKml)
	{
		start_point.lon = lon;
		start_point.lat = lat;
		start_point.alt = alt;
		WriteKMLini(kmlFile);
		iniWriteKml = false;
	}
	WriteKMLPath(kmlFile, lon, lat, alt, ts, q);

	last_point.lat = lat;
	last_point.lon = lon;
	last_point.alt = alt;
}

void CSkyTraqKml::RealPushOnePoint2(double lon, double lat, double alt, const F32* speed, const F32* degree, const CString& ts, QualityMode q)
{
	if(iniWriteKml)
	{
		start_point.lon = lon;
		start_point.lat = lat;
		start_point.alt = alt;
		WriteKMLini(kmlFile);
		iniWriteKml = false;
	}
	WriteKMLPath2(kmlFile, lon, lat, alt, speed, degree, ts, q);

	last_point.lat = lat;
	last_point.lon = lon;
	last_point.alt = alt;
}

static CString sTs = "";
static double sLon = 0;
static double sLat = 0;
static double sAlt = 0;
static F32 sSpeed = 0;
static F32 sDegree = 0;
static QualityMode sQm = Uninitial;
void EmptyPoint()
{
  sLon = sLat = sAlt = 0;
  sQm = Uninitial;
  sSpeed = sDegree = 0;
  sTs.Empty();
}

void SetPoint(double lon, double lat, double alt, const F32* speed, const F32* degree, const CString& ts, QualityMode q)
{
  sLon = lon;
  sLat = lat;
  sAlt = alt;
  if(speed)
  {
    sSpeed = *speed;
  }
  if(degree)
  {
    sDegree = *degree;
  }
  sQm = q;
  sTs = ts;
}

void CSkyTraqKml::PushOnePoint2(double lon, double lat, double alt, const F32* speed, const F32* degree, const CString& ts, QualityMode q)
{
  if(sTs.IsEmpty())
  { //The first point
    SetPoint(lon, lat, alt, speed, degree, ts, q);
    return;
  }

  if(ts.IsEmpty())
  { //The last point
    RealPushOnePoint2(sLon, sLat, sAlt, &sSpeed, &sDegree, sTs, sQm);
    return;
  }

  if(sTs != ts)
  {
    RealPushOnePoint2(sLon, sLat, sAlt, &sSpeed, &sDegree, sTs, sQm);
  }
  SetPoint(lon, lat, alt, speed, degree, ts, q);
}

void CSkyTraqKml::PushOnePoi(double lon, double lat, double alt)
{
	LL1 tmpLla;
	tmpLla.lat = lat;
	tmpLla.lon = lon;
	tmpLla.alt = alt;
	lst_poi.push_back(tmpLla);
}

void CSkyTraqKml::AddStartPoint(CFile& f)
{	
	CString str;
	str = "<Placemark id=\"Start Point\"><styleUrl>#POI_STYLE</styleUrl><name>Start Point</name><LookAt>";
	f.Write(str, (U16)str.GetLength());

	str.Format("<longitude>%012.9lf</longitude><latitude>%012.9lf</latitude></LookAt><Point>", start_point.lon, start_point.lat);
	f.Write(str, (U16)str.GetLength());

	str.Format("<coordinates>%012.9lf,%012.9lf</coordinates></Point></Placemark>",start_point.lon,start_point.lat);
	f.Write(str, (U16)str.GetLength());
}

void CSkyTraqKml::AddEndPoint(CFile& f)
{	
	CString str;
	str = "<Placemark id=\"End Point\"><styleUrl>#POI_STYLE</styleUrl><name>End Point</name><LookAt>";
	f.Write(str, (U16)str.GetLength());
	
	str.Format("<longitude>%012.9lf</longitude><latitude>%012.9lf</latitude></LookAt><Point>", last_point.lon, last_point.lat);
	f.Write(str, (U16)str.GetLength());

	str.Format("<coordinates>%012.9lf,%012.9lf</coordinates></Point></Placemark>", last_point.lon, last_point.lat),
	f.Write(str, (U16)str.GetLength());
}


void CSkyTraqKml::Finish()
{
	CString str;
	if(lst_poi.size() > 0)
	{
		str = "</coordinates></LineString></Placemark>";
		kmlFile.Write(str, (U16)str.GetLength());

		WritePOIPath(kmlFile, &lst_poi);

		str = "</kml>";
		kmlFile.Write(str, (U16)str.GetLength());

		lst_poi.clear();
	}
	else
	{
		if (!iniWriteKml)
		{
			str = "</coordinates></LineString></Placemark>";
			kmlFile.Write(str, (U16)str.GetLength());
		}

		if(pointList)
		{
			pls.AddTail(strPointList);
			POSITION p = pls.GetHeadPosition();
			for(int i = 0; i < pls.GetCount(); ++i)
			{
				str = pls.GetNext(p);
				kmlFile.Write(str, (U16)str.GetLength());
			}
			str = "</Folder>";
			kmlFile.Write(str, (U16)str.GetLength());
		}

		AddStartPoint(kmlFile);
		AddEndPoint(kmlFile);

		if (!iniWriteKml)
		{
			str = "</Folder></kml>";
			kmlFile.Write(str, (U16)str.GetLength());
		}
	}
	kmlFile.Close();
}

void CSkyTraqKml::Finish2()
{
  PushOnePoint2(0, 0, 0, NULL, NULL, "", Uninitial);
  EmptyPoint();
  Finish();
}

void CSkyTraqKml::WriteKMLini(CFile& f)
{
	/*<?xml version="1.0" encoding="UTF-8"?>
	<kml xmlns="http://earth.google.com/kml/2.1">*/
	CString str;
	str = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<kml xmlns=\"http://earth.google.com/kml/2.1\">";
	str += "<Style id=\"POI_STYLE\"><IconStyle><color>ff00ff00</color><width>2</width><scale>1.1</scale>";
	str += "<Icon><href>http://maps.google.com/mapfiles/kml/pal3/icon21.png</href></Icon></IconStyle>\r\n</Style>";
	str += "<Folder id=\"Data logger\"><name>Data logger</name>";
	str += "<Placemark id=\"logger\"><styleUrl>#lineStyle</styleUrl><description>Plot Your Traveling Path</description><name>Trajectory</name>";
	str += "<visibility>1</visibility><open>0</open><Style><LineStyle><color>ff";
	f.Write(str, (UINT)str.GetLength());
	str.Format("%06x", lineColor);
	f.Write(str, (UINT)str.GetLength());
	str = "</color></LineStyle></Style><LineString><extrude>0</extrude><tessellate>1</tessellate>";
	if(convert3d)
	{
		str += "<altitudeMode>absolute</altitudeMode>";
	}
	str += "<coordinates>";
	f.Write(str, (UINT)str.GetLength());
}

bool CompareByPRN(Satellite const& lhs, Satellite const& rhs)
{
	if(lhs.IsInUsePrn())
  {
		return lhs.GetPrn() < rhs.GetPrn();
  }
	return false;
}

char CSkyTraqKml::CheckGsa(int p, GPGSA *gsa)
{
	for(int i = 0; i < MAX_SATELLITE; ++i)
	{
		if(gsa->SatelliteID[i] == p)
			return 'O';
	}
	return 'X';
}

CString CSkyTraqKml::GenerateSatelliteTable(Satellites* s, GPGSA *gsa)
{
	//"<table class=\"tg\"><tr><th>PRN</th><th>Azimuth</th><th>Elevation</th><th>SNR</th><th>Used</th></tr>" \
	//"<tr><td>5</td><td>128</td><td>45</td><td>41</td><td>O</td></tr>" \
	//"<tr><td>6</td><td>315</td><td>89</td><td>40</td><td>X</td></tr>" \
	//"</table>";
	CString str = "<table class=\"tg\"><tr><th>PRN</th><th>AZI</th><th>ELE</th><th>SNR</th><th>Used</th></tr>";
	//Satellites* p = s;
	for(int i = 0; i < s->GetSateCount(); ++i)
	{
    const Satellite* sate = s->GetSateIndex(i);
		CString ss;
    F32 sn = 0;
		if(!IsValidateValue(s->GetSnrSigId(0)))
		{
			ss.Format("<tr><td>%d</td><td>%d</td><td>%d</td><td>--</td><td>%c</td></tr>", 
				sate->GetPrn(), sate->GetAzi(), sate->GetEle(), CheckGsa(sate->GetPrn(), gsa));
		}
		else
		{
			ss.Format("<tr><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%c</td></tr>", 
				sate->GetPrn(), sate->GetAzi(), sate->GetEle(), sn, CheckGsa(sate->GetPrn(), gsa));
		}
		str += ss;
	}
	str += "</table>";
	return str;
}

CString CSkyTraqKml::GetSatelliteInfo()
{
	CString str;
	if(satellites_gp && msg_gpgsa)
	{
		//std::sort(satellites_gp, (satellites_gp + MAX_SATELLITE), CompareByPRN);
		if(satellites_gp->GetSateCount() > 0)
		{
      satellites_gp->Sort();
			str += "GPS Satellites :<br>";
			str += GenerateSatelliteTable(satellites_gp, msg_gpgsa);
		}
	}
	if(satellites_gl && msg_glgsa)
	{
		//std::sort(satellites_gl, (satellites_gl + MAX_SATELLITE), CompareByPRN);
		if(satellites_gl->GetSateCount() > 0)
		{
#if(_NAVIC_CONVERT_)
			str += "NavIC Satellites :<br>";
#else
			str += "GLONASS Satellites :<br>";
#endif
      satellites_gp->Sort();
			str += GenerateSatelliteTable(satellites_gl, msg_glgsa);
		}
	}
	if(satellites_bd && msg_bdgsa)
	{
		//std::sort(satellites_bd, (satellites_bd + MAX_SATELLITE), CompareByPRN);
		if(satellites_bd->GetSateCount() > 0)
		{
      satellites_bd->Sort();
			str += "Beidou Satellites :<br>";
			str += GenerateSatelliteTable(satellites_bd, msg_bdgsa);
		}
	}
	if(satellites_ga && msg_gagsa)
	{
		//std::sort(satellites_ga, (satellites_ga + MAX_SATELLITE), CompareByPRN);
		if(satellites_ga->GetSateCount() > 0)
		{
      satellites_ga->Sort();
			str += "Galileo Satellites :<br>";
			str += GenerateSatelliteTable(satellites_ga, msg_gagsa);
		}
	}
	if(satellites_gi && msg_gigsa)
	{
		//std::sort(satellites_gi, (satellites_gi + MAX_SATELLITE), CompareByPRN);
		if(satellites_gi->GetSateCount() > 0)
		{
      satellites_gi->Sort();
			str += "Navic Satellites :<br>";
			str += GenerateSatelliteTable(satellites_gi, msg_gigsa);
		}
	}
	return str;
}

void CSkyTraqKml::WriteKMLPath(CFile& f, double lon, double lat, double alt, const CString& ts, QualityMode q)
{
	CString str;
	str = "      ";
	f.Write(str, str.GetLength());
	str.Format("%012.9lf,%012.9lf,%07.3lf\r\n", lon, lat, (convert3d) ? alt : 2.0);
	f.Write(str, str.GetLength());

	if(pointList)
	{
		CString str, qMode;
		if(q==FixRTK)
		{
			qMode = "Fix RTK";
		}
		else if(q==FloatRTK)
		{
			qMode = "Float RTK";
		}		
		else if(q==EstimatedMode)
		{
			qMode.Format("Estimated Mode");
		}
		else if(q==PositionFix2d)
		{
			qMode.Format("Position Fix 2D");
		}
		else if(q==PositionFix3d)
		{
			qMode.Format("Position Fix 3D");
		}
		else if(q==DgpsMode)
		{
			qMode.Format("DGPS");
		}
		else
		{
			qMode.Format("%d", q);
		}

		if(noPointText)
		{
			strPointList += "<Placemark><name></name><description><![CDATA[";
		}
		else
		{
			strPointList += "<Placemark><name>" + ts + "</name><description><![CDATA[";
		}
		str.Format("lontitude: %012.9lf <br>latitude: %012.9lf<br>altitude: %07.3lf<br>Time: %s<br>Fix Mode: %s<br>", lon, lat, alt, ts, qMode);
		strPointList += str;

		if(detailInfo)
		{
			strPointList += GetSatelliteInfo();
			//"<table class=\"tg\"><tr><th>PRN</th><th>Azimuth</th><th>Elevation</th><th>SNR</th><th>Used</th></tr><tr><td>5</td><td>128</td><td>45</td><td>41</td><td>O</td></tr><tr><td>6</td><td>315</td><td>89</td><td>40</td><td>X</td></tr></table>";
		}
		if(q==FixRTK)
		{
			strPointList += "]]></description><styleUrl>#PointStyle2</styleUrl><Point>";
		}
		else if(q==FloatRTK)
		{
			strPointList += "]]></description><styleUrl>#PointStyle3</styleUrl><Point>";
		}		
		else if(q==EstimatedMode)
		{
			strPointList += "]]></description><styleUrl>#PointStyle4</styleUrl><Point>";
		}		
		else
		{
			strPointList += "]]></description><styleUrl>#PointStyle</styleUrl><Point>";
		}
		if(convert3d)
		{
			strPointList += "<extrude>1</extrude><altitudeMode>absolute</altitudeMode>";
		}
		else
		{
			strPointList += "<altitudeMode>clampToGround</altitudeMode>";
		}
		str.Format("<coordinates>%012.9lf,%012.9lf,%07.3lf</coordinates>", lon, lat, alt);
		strPointList += str;
		strPointList += "</Point></Placemark>\r\n";
	
		if(strPointList.GetLength() > 4096)
		{
			pls.AddTail(strPointList);
			strPointList.Empty();
		}

	}
}

void CSkyTraqKml::WriteKMLPath2(CFile& f, double lon, double lat, double alt, const F32* speed, const F32* degree, const CString& ts, QualityMode q)
{
	CString str, qMode;
	str = "      ";
	f.Write(str, str.GetLength());
	str.Format("%012.9lf,%012.9lf,%07.3lf\r\n", lon, lat, (convert3d) ? alt : 2.0);
	f.Write(str, str.GetLength());

	if(!pointList)
	{
    return;
  }

	if(q==FixRTK)
	{
		qMode = "Fix RTK";
	}
	else if(q==FloatRTK)
	{
		qMode = "Float RTK";
	}		
	else if(q==EstimatedMode)
	{
		qMode.Format("Estimated Mode");
	}
	else if(q==PositionFix2d)
	{
		qMode.Format("Position Fix 2D");
	}
	else if(q==PositionFix3d)
	{
		qMode.Format("Position Fix 3D");
	}
	else if(q==DgpsMode)
	{
		qMode.Format("DGPS");
	}
	else
	{
		qMode.Format("%d", q);
	}

	if(noPointText)
	{
		strPointList += "<Placemark><name></name><description><![CDATA[";
	}
	else
	{
		strPointList += "<Placemark><name>" + ts + "</name><description><![CDATA[";
	}
  str.Format("LON: %012.9lf <br>LAT: %012.9lf<br>ALT: %07.3lf<br>Speed: %.2f <br>Direction:%.2f <br>Time: %s<br>Fix Mode: %s<br>", 
    lon, lat, alt, (speed) ? *speed * KNOTS2KMHR : 0, (degree) ? *degree : 0, ts, qMode);
	strPointList += str;

	if(detailInfo)
	{
		strPointList += GetSatelliteInfo();
		//"<table class=\"tg\"><tr><th>PRN</th><th>Azimuth</th><th>Elevation</th><th>SNR</th><th>Used</th></tr><tr><td>5</td><td>128</td><td>45</td><td>41</td><td>O</td></tr><tr><td>6</td><td>315</td><td>89</td><td>40</td><td>X</td></tr></table>";
	}
	if(q==FixRTK)
	{
		strPointList += "]]></description><styleUrl>#PointStyle2</styleUrl><Point>";
	}
	else if(q==FloatRTK)
	{
		strPointList += "]]></description><styleUrl>#PointStyle3</styleUrl><Point>";
	}		
	else if(q==EstimatedMode)
	{
		strPointList += "]]></description><styleUrl>#PointStyle4</styleUrl><Point>";
	}		
	else
	{
		strPointList += "]]></description><styleUrl>#PointStyle</styleUrl><Point>";
	}
	if(convert3d)
	{
		strPointList += "<extrude>1</extrude><altitudeMode>absolute</altitudeMode>";
	}
	else
	{
		strPointList += "<altitudeMode>clampToGround</altitudeMode>";
	}
	str.Format("<coordinates>%012.9lf,%012.9lf,%07.3lf</coordinates>", lon, lat, alt);
	strPointList += str;
	strPointList += "</Point></Placemark>\r\n";

	if(strPointList.GetLength() > 4096)
	{
		pls.AddTail(strPointList);
		strPointList.Empty();
	}

}

void CSkyTraqKml::WritePOIPath(CFile& f, vector<LL1> *lst)
{
	vector<LL1>::iterator iter;
	int id = 1;
	CString str;

	for(iter=lst->begin(); iter != lst->end(); ++iter)
	{
		LL1 lla = *iter;
		str.Format("<Placemark id=\"POI%d\"><styleUrl>#POI_STYLE</styleUrl><name>POI%d</name><LookAt>", id, id++);
		f.Write(str, str.GetLength());
		str.Format("<longitude>%012.9lf</longitude><latitude>%012.9lf</latitude></LookAt><Point>\r\n", lla.lon, lla.lat);
		f.Write(str, str.GetLength());
		str.Format("<coordinates>%012.9lf,%012.9lf</coordinates></Point></Placemark>", lla.lon, lla.lat);
		f.Write(str, str.GetLength());
	}
	str = "</Folder>\r\n";
	f.Write(str, str.GetLength());
}
