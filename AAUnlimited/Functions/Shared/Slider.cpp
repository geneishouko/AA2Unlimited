#include "Slider.h"


namespace Shared {

	namespace {
		struct Name {
			bool valid;
			char prefix;
			char extraPrefix;
			unsigned int num1;
			unsigned int num2;
			unsigned int num3;
			unsigned int num4;
			char suffix;

			Name(const char* name) : prefix(0), suffix(0), num1(0),num2(0),num3(0),num4(0) {
				valid = false;
				//two prefixes first
				prefix = *name;
				name++; if (!*name) return;
				if (!isdigit(*name)) {
					extraPrefix = *name;
					name++; if (!*name) return;
				}
				else {
					extraPrefix = '\0';
				}
				//now the 4 numbers
				const char* it = NULL;
				num1 = strtoul(name,(char**)&it,10);
				if (*it != '_') { return; }
				it++; if (!*it) return;
				num2 = strtoul(it,(char**)&it,10);
				if (*it != '_') { return; }
				it++; if (!*it) return;
				num3 = strtoul(it,(char**)&it,10);
				if (*it != '_') { return; }
				it++; if (!*it) return;
				num4 = strtoul(it,(char**)&it,10);
				if (*it) suffix = *it;
				valid = true;
			}
		};
	}

	ExtClass::CharacterStruct::Models Slider::GetModelFromName(const char* name) {
		//names we know:
		//FACE			A00_10_xx_00
		//SKELETON		A00_00_0x_00h
		//BODY			A00_00_0x_00
		//HAIR_FRONT	AS00_20_xx_yy (yy is flip)
		//HAIR_SIDE		AS00_21_xx_yy (yy is flip)
		//HAIR_BACK		AS00_22_xx_yy (yy is flip)
		//HAIR_EXT		AS00_23_xx_yy (yy is flip)
		//FACE_SLIDERS	<empty>
		if(name == NULL) return ExtClass::CharacterStruct::INVALID;
		Name props{ name };
		if (!props.valid) return ExtClass::CharacterStruct::INVALID;
		if (props.num1 != 0) return ExtClass::CharacterStruct::INVALID;
		if(props.extraPrefix == 'S') {
			//hairs
			switch(props.num2) {
			case 20:
				return ExtClass::CharacterStruct::HAIR_FRONT;
			case 21:
				return ExtClass::CharacterStruct::HAIR_SIDE;
			case 22:
				return ExtClass::CharacterStruct::HAIR_BACK;
			case 23:
				return ExtClass::CharacterStruct::HAIR_EXT;
			default:
				return ExtClass::CharacterStruct::INVALID;
			}
		}
		else if(props.extraPrefix == '\0') {
			//other stuff
			if (props.num2 == 10) return ExtClass::CharacterStruct::FACE;
			if (props.num2 == 0) {
				if(props.num3 >= 0 && props.num3 <= 2 && props.num4 == 0) {
					if (props.suffix == 'h') return ExtClass::CharacterStruct::SKELETON;
					else if (props.suffix == '\0') return ExtClass::CharacterStruct::BODY;
				}
			}
		}
		return ExtClass::CharacterStruct::INVALID;
		
	}

	void Slider::ModifySRT(D3DVECTOR3* scale,D3DVECTOR3* rot,D3DVECTOR3* trans,Slider::Operation op,const AAUCardData::BoneMod& mod) {
		switch (op) {
		case ADD:
			scale->x += mod.scales[0];
			scale->y += mod.scales[1];
			scale->z += mod.scales[2];
			trans->x += mod.transformations[0];
			trans->y += mod.transformations[1];
			trans->z += mod.transformations[2];
			rot->x += mod.rotations[0];
			rot->y += mod.rotations[1];
			rot->z += mod.rotations[2];
			break;
		case MULTIPLY:
			scale->x *= mod.scales[0];
			scale->y *= mod.scales[1];
			scale->z *= mod.scales[2];
			trans->x *= mod.transformations[0];
			trans->y *= mod.transformations[1];
			trans->z *= mod.transformations[2];
			rot->x += mod.rotations[0];
			rot->y += mod.rotations[1];
			rot->z += mod.rotations[2];
			break;
		case DIVIDE:
			if (mod.scales[0] != 0) scale->x /= mod.scales[0];
			if (mod.scales[1] != 0) scale->y /= mod.scales[1];
			if (mod.scales[2] != 0) scale->z /= mod.scales[2];
			if (mod.transformations[0] != 0) trans->x /= mod.transformations[0];
			if (mod.transformations[1] != 0)trans->y /= mod.transformations[1];
			if (mod.transformations[2] != 0)trans->z /= mod.transformations[2];
			rot->x += mod.rotations[0];
			rot->y += mod.rotations[1];
			rot->z += mod.rotations[2];
			break;
		default:
			break;
		}
	}

	void Slider::ModifyKeyframe(ExtClass::Keyframe* frame,Slider::Operation op,const AAUCardData::BoneMod& elem) {
		switch (op) {
		case ADD: {
			frame->m_scaleX += elem.scales[0];
			frame->m_scaleY += elem.scales[1];
			frame->m_scaleZ += elem.scales[2];
			D3DQUATERNION rotQuat;
			(*Shared::D3DXQuaternionRotationYawPitchRoll)(&rotQuat,elem.rotations[1],elem.rotations[0],elem.rotations[2]);
			D3DQUATERNION* origQuat = (D3DQUATERNION*)&frame->m_quatX;
			(*Shared::D3DXQuaternionMultiply)(origQuat,origQuat,&rotQuat);
			frame->m_transX += elem.transformations[0];
			frame->m_transY += elem.transformations[1];
			frame->m_transZ += elem.transformations[2];
			break; }
		case MULTIPLY: {
			frame->m_scaleX *= elem.scales[0];
			frame->m_scaleY *= elem.scales[1];
			frame->m_scaleZ *= elem.scales[2];
			D3DQUATERNION rotQuat;
			(*Shared::D3DXQuaternionRotationYawPitchRoll)(&rotQuat,elem.rotations[1],elem.rotations[0],elem.rotations[2]);
			D3DQUATERNION* origQuat = (D3DQUATERNION*)&frame->m_quatX;
			(*Shared::D3DXQuaternionMultiply)(origQuat,origQuat,&rotQuat);
			frame->m_transX *= elem.transformations[0];
			frame->m_transY *= elem.transformations[1];
			frame->m_transZ *= elem.transformations[2];
			break; }
		case DIVIDE: {
			if(elem.scales[0] != 0) frame->m_scaleX /= elem.scales[0];
			if (elem.scales[1] != 0) frame->m_scaleY /= elem.scales[1];
			if (elem.scales[2] != 0) frame->m_scaleZ /= elem.scales[2];
			D3DQUATERNION rotQuat;
			(*Shared::D3DXQuaternionRotationYawPitchRoll)(&rotQuat,elem.rotations[1],elem.rotations[0],elem.rotations[2]);
			D3DQUATERNION* origQuat = (D3DQUATERNION*)&frame->m_quatX;
			(*Shared::D3DXQuaternionMultiply)(origQuat,origQuat,&rotQuat);
			if (elem.transformations[0] != 0) frame->m_transX /= elem.transformations[0];
			if (elem.transformations[1] != 0) frame->m_transY /= elem.transformations[1];
			if (elem.transformations[2] != 0) frame->m_transZ /= elem.transformations[2];
			break; }
		default:
			break;
		}
	}


}