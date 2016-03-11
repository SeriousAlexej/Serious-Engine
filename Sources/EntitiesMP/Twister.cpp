/*
 * This file is generated by Entity Class Compiler, (c) CroTeam 1997-98
 */

#line 4 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"

#include "StdH.h"
#include "ModelsMP/Enemies/AirElemental/Twister.h"

#define ECF_TWISTER ( \
  ((ECBI_BRUSH|ECBI_MODEL|ECBI_CORPSE|ECBI_ITEM|ECBI_PROJECTILE_MAGIC|ECBI_PROJECTILE_SOLID)<<ECB_TEST) |\
  ((ECBI_MODEL|ECBI_CORPSE|ECBI_ITEM|ECBI_PROJECTILE_MAGIC|ECBI_PROJECTILE_SOLID)<<ECB_PASS) |\
  ((ECBI_MODEL)<<ECB_IS))
#define EPF_TWISTER ( \
  EPF_ONBLOCK_CLIMBORSLIDE|EPF_ORIENTEDBYGRAVITY|\
  EPF_TRANSLATEDBYGRAVITY|EPF_MOVABLE|EPF_ABSOLUTETRANSLATE)

#include <EntitiesMP/Twister.h>
#include <EntitiesMP/Twister_tables.h>
CEntityEvent *ETwister::MakeCopy(void) { CEntityEvent *peeCopy = new ETwister(*this); return peeCopy;}
ETwister::ETwister() : CEntityEvent(EVENTCODE_ETwister) {;
 ClearToDefault(penOwner);
 ClearToDefault(fSize);
 ClearToDefault(fDuration);
 ClearToDefault(sgnSpinDir);
 ClearToDefault(bGrow);
 ClearToDefault(bMovingAllowed);
};
#line 34 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"

static EntityInfo eiTwister = {
  EIBT_AIR, 0.0f,
  0.0f, 1.0f, 0.0f,
  0.0f, 0.75f, 0.0f,
};


#define MOVE_FREQUENCY 0.1f
#define ROTATE_SPEED 10000.0f
#define MOVE_SPEED 7.5f

void CTwister_OnPrecache(CDLLEntityClass *pdec, INDEX iUser) 
{
  pdec->PrecacheClass(CLASS_SPINNER);
  pdec->PrecacheModel(MODEL_TWISTER);
  pdec->PrecacheTexture(TEXTURE_TWISTER);
  pdec->PrecacheSound(SOUND_SPIN);
}

void CTwister::SetDefaultProperties(void) {
  m_penOwner = NULL;
  m_fSize = 1.0f;
  m_vSpeed = FLOAT3D(0 , 0 , 0);
  m_sgnSpinDir = 1;
  m_bGrow = TRUE ;
  m_tmLastMove = 0.0f;
  m_aSpeedRotation = FLOAT3D(0 , 0 , 0);
  m_bMoving = FALSE ;
  m_bMovingAllowed = TRUE ;
  m_vDesiredPosition = FLOAT3D(0 , 0 , 0);
  m_vDesiredAngle = FLOAT3D(0 , 0 , 0);
  m_fStopTime = 0.0f;
  m_fActionRadius = 0.0f;
  m_fActionTime = 0.0f;
  m_fDiffMultiply = 0.0f;
  m_fUpMultiply = 0.0f;
  m_bFadeOut = FALSE ;
  m_fFadeStartTime = 1e6;
  m_fFadeTime = 2.0f;
  m_fStartTime = 0.0f;
  m_soSpin.SetOwner(this);
m_soSpin.Stop_internal();
  CMovableModelEntity::SetDefaultProperties();
}
  
#line 97 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
void * CTwister::GetEntityInfo(void) {
#line 98 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
return & eiTwister ;
#line 99 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
  
#line 102 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
void CTwister::ReceiveDamage(CEntity * penInflictor,enum DamageType dmtType,
#line 103 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
FLOAT fDamageAmmount,const FLOAT3D & vHitPoint,const FLOAT3D & vDirection) 
#line 104 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
{
#line 105 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
return ;
#line 106 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
  
#line 109 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
void CTwister::RenderParticles(void) 
#line 110 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
{
#line 111 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
if(m_bMovingAllowed )
#line 112 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
{
#line 113 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
Particles_Twister  (this  , m_fSize  / 15.0f , m_fStartTime  , m_fFadeStartTime  , 1.0f);
#line 114 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
#line 115 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
else 
#line 116 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
{
#line 117 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
CEntity  * penParent  = GetParent  ();
#line 118 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
FLOAT fStretch  = 1.0f;
#line 119 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
if(penParent  != NULL )
#line 120 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
{
#line 121 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
CAirElemental  * penAir  = (CAirElemental  *) penParent ;
#line 122 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
FLOAT fStretchRatio  = penAir  -> GetCurrentStretchRatio  ();
#line 123 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
fStretch  = 1.0f + (fStretchRatio ) * 6.0f;
#line 124 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
#line 125 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
Particles_Twister  (this  , m_fSize  / 15.0f * fStretch  , m_fStartTime  , m_fFadeStartTime  , 0.5f * fStretch );
#line 126 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
#line 127 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
  
#line 132 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
BOOL CTwister::AdjustShadingParameters(FLOAT3D & vLightDirection,COLOR & colLight,COLOR & colAmbient) {
#line 134 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
if(m_bFadeOut ){
#line 135 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
FLOAT fTimeRemain  = m_fFadeStartTime  + m_fFadeTime  - _pTimer  -> CurrentTick  ();
#line 136 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
if(fTimeRemain  < 0.0f){fTimeRemain  = 0.0f;}
#line 137 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
COLOR colAlpha  = GetModelObject  () -> mo_colBlendColor ;
#line 138 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
colAlpha  = (colAlpha  & 0xffffff00) + (COLOR (fTimeRemain  / m_fFadeTime  * 0xff) & 0xff);
#line 139 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
GetModelObject  () -> mo_colBlendColor  = colAlpha ;
#line 140 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
#line 141 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
return CMovableModelEntity  :: AdjustShadingParameters  (vLightDirection  , colLight  , colAmbient );
#line 142 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
  
#line 149 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
void CTwister::SpinEntity(CEntity * pen) {
#line 152 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
if(IsOfClass  (pen  , "AirElemental") || IsOfClass  (pen  , "Twister") 
#line 153 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
|| IsDerivedFromClass  (pen  , "Item")){
#line 154 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
return ;
#line 155 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
#line 157 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
if(IsOfClass  (pen  , "Projectile")){
#line 158 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
if(((CProjectile  *) & * pen ) -> m_prtType  == PRT_AIRELEMENTAL_WIND )
#line 159 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
{
#line 160 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
return ;
#line 161 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
#line 162 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
#line 166 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
if(pen  -> GetPhysicsFlags  () & EPF_MOVABLE ){
#line 168 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
BOOL bNoSpinner  = TRUE ;
#line 169 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
{FOREACHINLIST  (CEntity  , en_lnInParent  , pen  -> en_lhChildren  , iten ){
#line 170 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
if(IsOfClass  (iten  , "Spinner"))
#line 171 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
{
#line 172 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
bNoSpinner  = FALSE ;
#line 173 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
return ;
#line 174 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
#line 175 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}}
#line 176 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
if(bNoSpinner ){
#line 177 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
ESpinnerInit  esi ;
#line 178 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
CEntityPointer penSpinner ;
#line 179 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . penParent  = pen ;
#line 180 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . penTwister  = this ;
#line 181 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . bImpulse  = FALSE ;
#line 184 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
if(IsOfClass  (pen  , "Projectile"))
#line 185 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
{
#line 186 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
switch(((CProjectile  &) * pen ) . m_prtType ){
#line 187 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
case PRT_GRENADE : 
#line 188 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
case PRT_HEADMAN_BOMBERMAN : 
#line 189 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
case PRT_DEMON_FIREBALL : 
#line 190 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
case PRT_SHOOTER_FIREBALL : 
#line 191 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
case PRT_BEAST_PROJECTILE : 
#line 192 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
case PRT_BEAST_BIG_PROJECTILE : 
#line 193 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
case PRT_LAVA_COMET : 
#line 194 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . tmSpinTime  = 2.5f;
#line 195 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . vRotationAngle  = ANGLE3D (- m_sgnSpinDir  * 250.0f , 0 , 0);
#line 196 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . fUpSpeed  = m_fDiffMultiply  * 0.75;
#line 197 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
break ;
#line 198 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
default  : 
#line 199 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . tmSpinTime  = 1.5f;
#line 200 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . vRotationAngle  = ANGLE3D (- m_sgnSpinDir  * 180.0f , 0 , 0);
#line 201 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . fUpSpeed  = m_fDiffMultiply  / 5.0f;
#line 202 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
break ;
#line 203 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
#line 205 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}else if(IsOfClass  (pen  , "Cannon ball")){
#line 206 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . tmSpinTime  = 0.2f;
#line 207 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . vRotationAngle  = ANGLE3D (- m_sgnSpinDir  * 500.0f , 0 , 0);
#line 208 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . fUpSpeed  = m_fDiffMultiply  * 3.0f;
#line 210 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}else if(IsOfClass  (pen  , "Player")){
#line 211 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . tmSpinTime  = 3.0f;
#line 212 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . vRotationAngle  = ANGLE3D (- m_sgnSpinDir  * 220.0f , 0 , 0);
#line 213 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . bImpulse  = TRUE ;
#line 214 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . fUpSpeed  = m_fDiffMultiply  * (0.4f + FRnd  () * 0.4f);
#line 215 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . tmImpulseDuration  = 1.4f + FRnd  () * 0.5f;
#line 217 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}else {
#line 218 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . tmSpinTime  = 0.5f;
#line 219 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . vRotationAngle  = ANGLE3D (- m_sgnSpinDir  * 180.0f , 0 , 0);
#line 220 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
esi  . fUpSpeed  = m_fDiffMultiply ;
#line 221 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
#line 222 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
penSpinner  = CreateEntity  (pen  -> GetPlacement  () , CLASS_SPINNER );
#line 223 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
penSpinner  -> Initialize  (esi );
#line 224 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
penSpinner  -> SetParent  (pen );
#line 225 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
#line 227 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
FLOAT3D vDirection ;
#line 228 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
AnglesToDirectionVector  (GetPlacement  () . pl_OrientationAngle  , vDirection );
#line 229 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
InflictDirectDamage  (pen  , m_penOwner  , DMT_IMPACT  , 2.0f , GetPlacement  () . pl_PositionVector  , vDirection );
#line 230 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
#line 232 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
  
#line 234 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
void CTwister::PreMoving(void) {
#line 236 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
if(m_bMoving ){
#line 237 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
FLOATmatrix3D m ;
#line 238 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
ANGLE3D aRotation ;
#line 239 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
aRotation  = m_aSpeedRotation  * (_pTimer  -> CurrentTick  () - m_tmLastMove );
#line 240 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
MakeRotationMatrix  (m  , aRotation );
#line 241 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_vSpeed  = m_vSpeed  * m ;
#line 242 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
SetDesiredTranslation  (m_vSpeed );
#line 243 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_tmLastMove  = _pTimer  -> CurrentTick  ();
#line 244 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
#line 245 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
CMovableModelEntity  :: PreMoving  ();
#line 246 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
BOOL CTwister::
#line 254 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
Main(const CEntityEvent &__eeInput) {
#undef STATE_CURRENT
#define STATE_CURRENT STATE_CTwister_Main
  ASSERTMSG(__eeInput.ee_slEvent==EVENTCODE_ETwister, "CTwister::Main expects 'ETwister' as input!");  const ETwister &et = (const ETwister &)__eeInput;
#line 256 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
ASSERT  (et  . penOwner  != NULL );
#line 257 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_penOwner  = et  . penOwner ;
#line 258 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_sgnSpinDir  = et  . sgnSpinDir ;
#line 259 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
if(m_sgnSpinDir  == 0){m_sgnSpinDir  = 1;};
#line 260 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_fSize  = et  . fSize ;
#line 261 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_fStopTime  = _pTimer  -> CurrentTick  () + et  . fDuration ;
#line 262 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_bGrow  = et  . bGrow ;
#line 263 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_bMovingAllowed  = et  . bMovingAllowed ;
#line 266 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
InitAsEditorModel  ();
#line 267 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
SetPhysicsFlags  (EPF_TWISTER );
#line 268 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
SetCollisionFlags  (ECF_TWISTER );
#line 269 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
SetFlags  (GetFlags  () | ENF_SEETHROUGH );
#line 270 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
SetModel  (MODEL_TWISTER );
#line 271 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
SetModelMainTexture  (TEXTURE_TWISTER );
#line 274 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_fActionRadius  = pow  (m_fSize  , 0.33333f) * 10.0f;
#line 275 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_fActionTime  = m_fActionRadius ;
#line 276 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_fUpMultiply  = m_fActionRadius  / 2.0f;
#line 277 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_fDiffMultiply  = sqrt  (m_fSize );
#line 278 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
GetModelObject  () -> StretchModel  (FLOAT3D (m_fSize  , m_fSize  , m_fSize ));
#line 279 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
ModelChangeNotify  ();
#line 281 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_fStartTime  = _pTimer  -> CurrentTick  ();
#line 284 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
SetTimerAfter(FRnd  () * 0.25f);
Jump(STATE_CURRENT, 0x01fb0001, FALSE, EBegin());return TRUE;}BOOL CTwister::H0x01fb0001_Main_01(const CEntityEvent &__eeInput) {
#undef STATE_CURRENT
#define STATE_CURRENT 0x01fb0001
switch(__eeInput.ee_slEvent) {case EVENTCODE_EBegin: return TRUE;case EVENTCODE_ETimer: Jump(STATE_CURRENT,0x01fb0002, FALSE, EInternal()); return TRUE;default: return FALSE; }}BOOL CTwister::H0x01fb0002_Main_02(const CEntityEvent &__eeInput){
ASSERT(__eeInput.ee_slEvent==EVENTCODE_EInternal);
#undef STATE_CURRENT
#define STATE_CURRENT 0x01fb0002
;
#line 286 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_soSpin  . Set3DParameters  (50.0f , 10.0f , 1.0f , 1.0f);
#line 287 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
PlaySound  (m_soSpin  , SOUND_SPIN  , SOF_3D  | SOF_LOOP );
#line 290 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
SetDesiredRotation  (ANGLE3D (m_sgnSpinDir  * (FRnd  () * 50.0f + 50.0f) , 0.0f , 0.0f));
#line 292 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
if(m_bGrow ){
#line 293 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
StartModelAnim  (TWISTER_ANIM_GROWING  , AOF_SMOOTHCHANGE  | AOF_NORESTART );
#line 294 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
#line 295 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
SetTimerAfter(GetModelObject  () -> GetAnimLength  (TWISTER_ANIM_GROWING ));
Jump(STATE_CURRENT, 0x01fb0003, FALSE, EBegin());return TRUE;}BOOL CTwister::H0x01fb0003_Main_03(const CEntityEvent &__eeInput) {
#undef STATE_CURRENT
#define STATE_CURRENT 0x01fb0003
switch(__eeInput.ee_slEvent) {case EVENTCODE_EBegin: return TRUE;case EVENTCODE_ETimer: Jump(STATE_CURRENT,0x01fb0004, FALSE, EInternal()); return TRUE;default: return FALSE; }}BOOL CTwister::H0x01fb0004_Main_04(const CEntityEvent &__eeInput){
ASSERT(__eeInput.ee_slEvent==EVENTCODE_EInternal);
#undef STATE_CURRENT
#define STATE_CURRENT 0x01fb0004
;
#line 298 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
FLOAT fR  = FRndIn  (5.0f , 10.0f);
#line 299 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
FLOAT fA  = FRnd  () * 360.0f;
#line 300 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_vSpeed  = FLOAT3D (CosFast  (fA ) * fR  , 0 , SinFast  (fA ) * fR );
#line 301 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_bMoving  = m_bMovingAllowed ;
#line 304 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
Jump(STATE_CURRENT,0x01fb0007, FALSE, EInternal());return TRUE;}BOOL CTwister::H0x01fb0007_Main_07(const CEntityEvent &__eeInput){
ASSERT(__eeInput.ee_slEvent==EVENTCODE_EInternal);
#undef STATE_CURRENT
#define STATE_CURRENT 0x01fb0007
if(!(_pTimer  -> CurrentTick  () < m_fStopTime )){ Jump(STATE_CURRENT,0x01fb0008, FALSE, EInternal());return TRUE;}
#line 305 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
FLOAT fMoveTime  = FRndIn  (2.0f , 4.0f);
#line 306 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_aSpeedRotation  = FLOAT3D (FRndIn  (8.0f , 16.0f) , 0.0f , 0.0f);
#line 307 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_tmLastMove  = _pTimer  -> CurrentTick  ();
#line 310 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
SetTimerAfter(fMoveTime );
Jump(STATE_CURRENT, 0x01fb0005, FALSE, EBegin());return TRUE;}BOOL CTwister::H0x01fb0005_Main_05(const CEntityEvent &__eeInput) {
#undef STATE_CURRENT
#define STATE_CURRENT 0x01fb0005
switch(__eeInput.ee_slEvent){case(EVENTCODE_EBegin):{const EBegin&e= (EBegin&)__eeInput;
return TRUE;}ASSERT(FALSE);break;case(EVENTCODE_ETimer):{const ETimer&e= (ETimer&)__eeInput;
UnsetTimer();Jump(STATE_CURRENT,0x01fb0006, FALSE, EInternal());return TRUE;}ASSERT(FALSE);break;case(EVENTCODE_EPass):{const EPass&ep= (EPass&)__eeInput;

#line 314 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
if(ep  . penOther  -> GetRenderType  () & RT_MODEL  && 
#line 315 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
ep  . penOther  -> GetPhysicsFlags  () & EPF_MOVABLE  && 
#line 316 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
! IsOfClass  (ep  . penOther  , "Twister")){
#line 317 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
SpinEntity  (ep  . penOther );
#line 318 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}
#line 319 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
return TRUE;
#line 320 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}ASSERT(FALSE);break;default: return FALSE; break;
#line 321 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}return TRUE;}BOOL CTwister::H0x01fb0006_Main_06(const CEntityEvent &__eeInput){
ASSERT(__eeInput.ee_slEvent==EVENTCODE_EInternal);
#undef STATE_CURRENT
#define STATE_CURRENT 0x01fb0006
Jump(STATE_CURRENT,0x01fb0007, FALSE, EInternal());return TRUE;
#line 322 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
}BOOL CTwister::H0x01fb0008_Main_08(const CEntityEvent &__eeInput) {
ASSERT(__eeInput.ee_slEvent==EVENTCODE_EInternal);
#undef STATE_CURRENT
#define STATE_CURRENT 0x01fb0008

#line 325 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_fFadeStartTime  = _pTimer  -> CurrentTick  ();
#line 326 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_bFadeOut  = TRUE ;
#line 327 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
m_fFadeTime  = 2.0f;
#line 328 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
SetTimerAfter(m_fFadeTime );
Jump(STATE_CURRENT, 0x01fb0009, FALSE, EBegin());return TRUE;}BOOL CTwister::H0x01fb0009_Main_09(const CEntityEvent &__eeInput) {
#undef STATE_CURRENT
#define STATE_CURRENT 0x01fb0009
switch(__eeInput.ee_slEvent) {case EVENTCODE_EBegin: return TRUE;case EVENTCODE_ETimer: Jump(STATE_CURRENT,0x01fb000a, FALSE, EInternal()); return TRUE;default: return FALSE; }}BOOL CTwister::H0x01fb000a_Main_10(const CEntityEvent &__eeInput){
ASSERT(__eeInput.ee_slEvent==EVENTCODE_EInternal);
#undef STATE_CURRENT
#define STATE_CURRENT 0x01fb000a
;
#line 331 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
Destroy  ();
#line 333 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
Return(STATE_CURRENT,EVoid());
#line 333 "D:/SE1_GPL/Sources/EntitiesMP/Twister.es"
return TRUE; ASSERT(FALSE); return TRUE;};