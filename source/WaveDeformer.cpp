/* Wave Deformer for Cinema 4D
 * 
 * Copyright (C) 2016 Johannes Kollender - All Rights Reserved
 * You may use, distribute and modify this code under the
 * terms of the AGPL-3.0 license.
 *
 * You should have received a copy of the AGPL-3.0 license with
 * this file. If not, please visit:
 * https://github.com/jx1k/Wave-Deformer/blob/main/LICENSE
 */

#include "c4d.h"
#include "c4d_symbols.h"
#include "Owavedeformer.h"
#include "xslanoise.h"
#include "xslagradient.h"
#include <iostream>

#define HANDLE_CNT 3
#define SPLINE_CNT 200

double min(double a,double b) {return (a<b)?a:b;}
double max(double a,double b) {return (a>b)?a:b;}

inline Real RangeMap(Real value, Real min_input, Real max_input, Real min_output, Real max_output)
{
	if (value<min_input) return min_output;
    if (value>max_input) return max_output;
	Real inrange = max_input - min_input;
	if (CompareFloatTolerant(inrange, RCO 0.0)) value = RCO 0.0;  // Prevent DivByZero error
	else value = (value - min_input) / inrange;             // Map input range to [0.0 ... 1.0]
	return  min_output + (max_output - min_output) * value; // Map to output range and return result
}

class WaveDeformer : public ObjectData
{
	BaseShader	*texture;
	BaseShader *mask;
	ChannelData cd;
	void DrawLine(BaseObject *op, BaseDraw *bd, Vector p1, Vector p2);

	public:
		virtual Bool Init(GeListNode *node);
		virtual Bool Message(GeListNode *node, LONG type, void *data);
		virtual void GetDimension (BaseObject *op, Vector *mp, Vector *rad);
		virtual DRAWRESULT Draw	(BaseObject *op, DRAWPASS type, BaseDraw *bd, BaseDrawHelp *bh);
		virtual void GetHandle(BaseObject *op, LONG i, HandleInfo &info);
		virtual LONG DetectHandle (BaseObject *op, BaseDraw *bd, LONG x, LONG y, QUALIFIER qualifier);
		virtual Bool MoveHandle	(BaseObject *op, BaseObject *undo, const Vector &mouse_pos, LONG hit_id, QUALIFIER qualifier, BaseDraw *bd);
		virtual Bool ModifyObject(BaseObject *op, BaseDocument *doc, BaseObject *mod, const Matrix &op_mg, const Matrix &mod_mg, Real lod, LONG flags, BaseThread *thread);

		static NodeData *Alloc(void) { return gNew WaveDeformer; }
};

Bool WaveDeformer::Message(GeListNode *node, LONG type, void *data)
{
	if (type==MSG_MENUPREPARE)
	{
		((BaseObject*)node)->SetDeformMode(TRUE);
	}
	return TRUE;
}

void WaveDeformer::GetDimension(BaseObject *op, Vector *mp, Vector *rad)
{
	BaseContainer *data = op->GetDataInstance(); 
	*mp = 0.0;
	*rad = Vector(data->GetReal(WAVE_WIDTH)*2);
}

void WaveDeformer::DrawLine(BaseObject *op, BaseDraw *bd, Vector p1, Vector p2) {
	BaseContainer *data = op->GetDataInstance();
	Vector step = (p2-p1).GetNormalized() * ((p2-p1).GetLength()/(SPLINE_CNT-1));
	LONG i;
	//Get variables from data
	Real twist=data->GetReal(WAVE_TWIST_AMOUNT);
	Real width=data->GetReal(WAVE_WIDTH);
	Real bend = data->GetReal(WAVE_BEND);
	Gradient *width_gradient = (Gradient*)data->GetCustomDataType(WAVE_FALLOFF,CUSTOMDATATYPE_GRADIENT);
	width_gradient->InitRender(InitRenderStruct());

	Matrix matrix;
	Vector p;
	Vector p_prev = NULL;
	Real angle, length, amount;

	for (i=0; i<SPLINE_CNT; i++)
	{
		//Calc matrix
		p =  p1+(step*i);
        angle = atan2(p.z, p.x);
		matrix = MatrixRotY(angle * bend) * MatrixMove(Vector(0,0,angle * bend));
		//Calc point
		length = sqrt(p.x*p.x + p.y*p.y);
		angle = atan2(p.x,p.y);
		amount = 1.0;
		if (width != 0) amount = min(1,max(0,length/width));
		amount = RGBToHSV(width_gradient->CalcGradientPixel(amount)).z; //Get value from gradient
		if (length != 0) angle += amount * twist/length;

		p = Vector(sin(angle)*length,cos(angle)*length,p.z);
		if (p_prev!=NULL) bd->DrawLine(p_prev, p, 0);
		p_prev = p;
	}
}

DRAWRESULT WaveDeformer::Draw(BaseObject *op, DRAWPASS drawpass, BaseDraw *bd, BaseDrawHelp *bh)
{
	if (drawpass==DRAWPASS_OBJECT)
	{ 
		BaseContainer *data = op->GetDataInstance(); 
		Real width=data->GetReal(WAVE_WIDTH);
		Real length=data->GetReal(WAVE_LENGTH);
		Vector h;
		Matrix m = bh->GetMg();

		m.v1*=width;
		m.v2*=width;
		m.v3*=width;
		
		bd->SetMatrix_Matrix(NULL, op->GetMg(), 0);
		bd->SetPen(bd->GetObjectColor(bh,op));
		
		if (data->GetReal(WAVE_PREVIEW)) {
			DrawLine(op, bd, Vector(-width,0,length), Vector(width,0,length));
			DrawLine(op, bd, Vector(-width,0,-length), Vector(width,0,-length));
			DrawLine(op, bd, Vector(width,0,-length), Vector(width,0,length));
			DrawLine(op, bd, Vector(-width,0,-length), Vector(-width,0,length));
		}
		if (data->GetReal(WAVE_SIZE_PREVIEW)) {
			bd->DrawLine(Vector(-width,0,length), Vector(width,0,length), 0);
			bd->DrawLine(Vector(-width,0,-length), Vector(width,0,-length), 0);
			bd->DrawLine(Vector(width,0,-length), Vector(width,0,length), 0);
			bd->DrawLine(Vector(-width,0,-length), Vector(-width,0,length), 0);
		}
	}
	else if (drawpass==DRAWPASS_HANDLES)
	{
		LONG   i;
		LONG    hitid = op->GetHighlightHandle(bd);
		HandleInfo info;

		bd->SetPen(GetViewColor(VIEWCOLOR_ACTIVEPOINT));
		bd->SetMatrix_Matrix(op, bh->GetMg());
		for (i=0; i<HANDLE_CNT; i++)
		{
			GetHandle(op, i, info);
			if (hitid==i)
				bd->SetPen(GetViewColor(VIEWCOLOR_SELECTION_PREVIEW));
			else
				bd->SetPen(GetViewColor(VIEWCOLOR_ACTIVEPOINT));
			bd->DrawHandle(info.position,DRAWHANDLE_BIG, 0);
			bd->SetPen(GetViewColor(VIEWCOLOR_ACTIVEPOINT));
			bd->DrawLine(info.position, Vector(0.0), 0);
		}
	}
	return DRAWRESULT_OK;
}

void WaveDeformer::GetHandle(BaseObject *op, LONG i, HandleInfo &info)
{
	BaseContainer *data = op->GetDataInstance();
	if (!data) return;

	switch (i)
	{
	case 0: 
		info.position = Vector(data->GetReal(WAVE_WIDTH), 0.0, 0.0);
		info.direction = Vector(1.0,0.0,0.0);
		info.type = HANDLECONSTRAINTTYPE_LINEAR;
		break;

	case 1:
		info.position = Vector(0.0, 0.0, data->GetReal(WAVE_LENGTH));
		info.direction = Vector(0.0,0.0,1.0);
		info.type = HANDLECONSTRAINTTYPE_LINEAR;
		break;

	case 2:
		info.position = Vector(0.0, data->GetReal(WAVE_TWIST_AMOUNT), 0.0);
		info.direction = Vector(0.0,1.0,0.0);
		info.type = HANDLECONSTRAINTTYPE_LINEAR;

	default: break;
	}
}

LONG WaveDeformer::DetectHandle(BaseObject *op, BaseDraw *bd, LONG x, LONG y, QUALIFIER qualifier)
{
	if (qualifier&QUALIFIER_CTRL) return NOTOK;

	HandleInfo info;
	Matrix	mg = op->GetMg();
	LONG    i,ret=NOTOK;
	Vector	p;

	for (i=0; i<HANDLE_CNT; i++)
	{
		GetHandle(op, i, info);
		if (bd->PointInRange(info.position*mg,x,y)) 
		{
			ret=i;
			if (!(qualifier&QUALIFIER_SHIFT)) break;
		}
	}
	return ret;
}

Bool WaveDeformer::MoveHandle(BaseObject *op, BaseObject *undo, const Vector &mouse_pos, LONG hit_id, QUALIFIER qualifier, BaseDraw *bd)
{
	BaseContainer *dst = op->GetDataInstance();
	HandleInfo info;
	
	Real val = mouse_pos.x;
	GetHandle(op, hit_id, info);

	if (bd)
	{
		Matrix mg = op->GetUpMg() * undo->GetMl();
		Vector pos = bd->ProjectPointOnLine(info.position * mg, info.direction ^ mg, mouse_pos.x, mouse_pos.y);
		val = (pos * !mg) * info.direction;
	}

	switch (hit_id)
	{
		case 0: 
			dst->SetReal(WAVE_WIDTH,FCut(val,RCO 0.0,RCO MAXRANGE)); 
			break;

		case 1:
			dst->SetReal(WAVE_LENGTH,FCut(val, RCO 0.0,RCO MAXRANGE)); 
			break;
		
		case 2: 
			dst->SetReal(WAVE_TWIST_AMOUNT,FCut(val, RCO 0.0,RCO MAXRANGE)); 
			break;

		default:
			break;
	}
	return TRUE;
}

Bool WaveDeformer::ModifyObject(BaseObject *mod, BaseDocument *doc, BaseObject *op, const Matrix &op_mg, const Matrix &mod_mg, Real lod, LONG flags, BaseThread *thread)
{
	if (!op->IsInstanceOf(Opoint)) return TRUE;
	BaseContainer *data = mod->GetDataInstance(); 

	// Get geometry data
	Vector  p,*padr=NULL;
	Matrix matrix;
	LONG i,pcnt;
	Real s;
	SReal *weight=NULL;
	padr = ToPoint(op)->GetPointW();
	pcnt = ToPoint(op)->GetPointCount(); if (!pcnt) return TRUE;
	weight = ToPoint(op)->CalcVertexMap(mod);

	//Get variables from data
	Real twist=data->GetReal(WAVE_TWIST_AMOUNT);
	Real width=data->GetReal(WAVE_WIDTH);
	Real length=data->GetReal(WAVE_LENGTH);
	Real bend = data->GetReal(WAVE_BEND);
	texture = (BaseShader*)data->GetLink(WAVE_TEXTURE,doc,Xbase);
	if (texture) {
		if (texture->InitRender(InitRenderStruct()) != INITRENDERRESULT_OK) return FALSE;
	}
	Vector texture_offset = data->GetVector(WAVE_TEXTURE_OFFSET);
	Gradient *width_gradient = (Gradient*)data->GetCustomDataType(WAVE_FALLOFF,CUSTOMDATATYPE_GRADIENT);
	width_gradient->InitRender(InitRenderStruct());

	bool use_length_falloff = data->GetBool(WAVE_USE_LENGTH_FALLOFF);
	Gradient *length_gradient = (Gradient*)data->GetCustomDataType(WAVE_LENGTH_FALLOFF,CUSTOMDATATYPE_GRADIENT);
	if (use_length_falloff) {
		length_gradient->InitRender(InitRenderStruct());
	}
	
	bool use_mask = data->GetBool(WAVE_USE_MASK);
	if (use_mask) {
		mask = (BaseShader*)data->GetLink(WAVE_MASK,doc,Xbase);
		if (mask) {
			if (mask->InitRender(InitRenderStruct()) != INITRENDERRESULT_OK) return FALSE;
		}
	}

	Real angle, dist, amount, x, z;

	for (i=0; i<pcnt; i++)
	{
		if (thread && !(i&63) && thread->TestBreak()) break;

		//Calc matrix
		p =  padr[i];
        angle = atan2(p.z, p.x);
		matrix = MatrixRotY(angle * bend) * mod->GetMl() * MatrixMove(Vector(0,0,angle * bend));

		//Calc point
		p = !matrix * p;
		dist = sqrt(p.x*p.x + p.y*p.y);
		angle = atan2(p.x,p.y);
		amount = 1.0;
		if (width != 0) amount = min(1,max(0,dist/width));

		amount = RGBToHSV(width_gradient->CalcGradientPixel(amount)).z; //Get value from gradient
		
		if (texture) {
			cd.p = Vector(p.z*0.002,amount*0.01,0) + texture_offset*0.01;
			amount *= RGBToHSV(texture->Sample(&cd)).z; //Get value from shader
		}

		z = RangeMap(p.z, -length, length, 1.0, 0.0);

		if (use_length_falloff) amount *= length_gradient->CalcGradientPixel(z).x;

		if (use_mask) {
			x = RangeMap(p.x, -width, width, 1.0, 0.0);
			cd.p = Vector(z, x, 0);
			amount *= RGBToHSV(mask->Sample(&cd)).z;
		}

		if (dist != 0) angle += amount * twist/dist;

		padr[i] = matrix * Vector(sin(angle)*dist,cos(angle)*dist,p.z);
	}

	// Free texture and shader
	width_gradient->FreeRender();
	if (use_length_falloff) length_gradient->FreeRender();
    if (texture) texture->FreeRender();
	if (mask) mask->FreeRender();

	GeFree(weight);
	op->Message(MSG_UPDATE);
	
	return TRUE;
}

Bool WaveDeformer::Init(GeListNode *node)
{	
	BaseObject *op   = (BaseObject*)node;
	BaseContainer *data = op->GetDataInstance();

	data->SetReal(WAVE_TWIST_AMOUNT, 8.6);
	data->SetReal(WAVE_WIDTH, 7.0);
	data->SetReal(WAVE_LENGTH, 10.0);
	data->SetVector(WAVE_TEXTURE_OFFSET, Vector(8.8, -3.129, 0));
	data->SetReal(WAVE_BEND, 0.0);
	data->SetBool(WAVE_PREVIEW, TRUE);
	data->SetBool(WAVE_SIZE_PREVIEW, FALSE);
	//Set noise shader
	BaseShader *wavenoise = BaseShader::Alloc(Xnoise);
	wavenoise->SetParameter(DescID(SLA_NOISE_NOISE), GeData(2008), DESCFLAGS_SET_0);
	wavenoise->SetParameter(DescID(SLA_NOISE_OCTAVES), GeData(8), DESCFLAGS_SET_0);
	if (wavenoise) {
		data->SetLink(WAVE_TEXTURE, wavenoise);
		op->InsertShader(wavenoise);
		}

	//Set gradient shader in mask
	AutoAlloc<Gradient> gradient;
	if (!gradient) return FALSE;
	GradientKnot k1,k2,k3;

	k1.col = Vector(1.0);
	k1.pos = 0.45;
	k2.col = Vector(0.0);
	k2.pos = 0.92;

	gradient->InsertKnot(k1);
	gradient->InsertKnot(k2);

	BaseShader *maskgradient = BaseShader::Alloc(Xgradient);
	maskgradient->SetParameter(DescID(SLA_GRADIENT_TYPE), GeData(SLA_GRADIENT_TYPE_2D_CIRC), DESCFLAGS_SET_0);
	maskgradient->SetParameter(DescID(SLA_GRADIENT_TURBULENCE), GeData(0.14), DESCFLAGS_SET_0);
	maskgradient->SetParameter(DescID(SLA_GRADIENT_OCTAVES), GeData(2), DESCFLAGS_SET_0);
	maskgradient->SetParameter(DescID(SLA_GRADIENT_GRADIENT), GeData(CUSTOMDATATYPE_GRADIENT, gradient), DESCFLAGS_SET_0);
	if (maskgradient) {
		data->SetLink(WAVE_MASK, maskgradient);
		op->InsertShader(maskgradient);
		}

	// Set gradient
	gradient->FlushKnots();
	k1.col = Vector(0.5102);
	k1.pos = 0.0;
	k2.col = Vector(1.0);
	k2.pos = 0.2658;
	k3.col = Vector(0.0);
	k3.pos = 1.0;

	gradient->InsertKnot(k1);
	gradient->InsertKnot(k2);
	gradient->InsertKnot(k3);

	data->SetData(WAVE_FALLOFF,GeData(CUSTOMDATATYPE_GRADIENT,gradient));

	gradient->FlushKnots();
	k1.col = Vector(0.0);
	k1.pos = 0.0;
	k2.col = Vector(1.0);
	k2.pos = 0.26;
	k3.col = Vector(0.0);
	k3.pos = 1.0;

	gradient->InsertKnot(k1);
	gradient->InsertKnot(k2);
	gradient->InsertKnot(k3);

	data->SetData(WAVE_LENGTH_FALLOFF, GeData(CUSTOMDATATYPE_GRADIENT,gradient));

	// Init channel data
	cd.n = Vector(0, 0, 1);
	cd.d = Vector(0, 0, 0);
	cd.t = 0.0;
	cd.texflag = 0;
	cd.vd = NULL;
	cd.off = 0.0;
	cd.scale = 0.0;

	return TRUE;
}

// Unique ID obtained from www.plugincafe.com
#define ID_WAVEDEFORMER	1031983

Bool RegisterWaveDeformer(void)
{
	// decide by name if the plugin shall be registered - just for user convenience
	String name=GeLoadString(IDS_WAVEDEFORMER); if (!name.Content()) return TRUE;
	return RegisterObjectPlugin(ID_WAVEDEFORMER, name, OBJECT_MODIFIER, WaveDeformer::Alloc,"Owavedeformer", AutoBitmap("wd_icon.tif"), 0);
}