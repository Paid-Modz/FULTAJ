#include "SDK.hpp"
using namespace SDK;
using namespace std;
#include <android/native_window_jni.h>
#include <android/log.h>
#include "Main/Includes.h"
#include "Main/Vector3.hpp"
#include "Main/Vector2.hpp"
#include "Main/MemoryTools.h"
#include "Main/KittyMemory/MemoryPatch.h"
#include "Main/android_native_app_glue.h"
#include "Main/obfuscate.h"
#include "Main/FULTAJ_GUI.h"
#include <thread>
#include <chrono>
#include "Main/And64InlineHook.hpp" 
#include "Main/Tools.h"
#include "Main/oxorany.cpp"
android_app *g_App = 0;
////==========================================================================================================//
std::string Version;// = "Version ( v3.8.0 )";
std::string DevName = oxorany("Play Safe Avoid Reports");
std::string loadername = oxorany("LOADERxBGMI~MagicBullet_x64");
#define COLOR_TEXT FLinearColor(1.f, 1.f, 1.f, 1.f) //text
#define COLOR_BONE FLinearColor(1.f, 0.f, 0.f, 1.f) //red
#define COLOR_BONE_VIS FLinearColor(0.f, 1.f, 0.f, 1.f) //visible skeletone
#define COLOR_HEALTH FLinearColor(0.f, 1.f, 1.f, 1.f) //sky Blue
bool fultaj = true;
bool CrashFixer = true;
float THIKNESS = 1.0;
ANativeWindow *ANativeWindow_p;
void android_main(struct android_app* state){
}
////==========================================================================================================//
#define PI 3.14159265358979323846
#define RAD2DEG(x) ((float)(x) * (float)(180.f / PI))
#define Actors_Offset 0xA0
#define GNames_Offset 0x78a0af8
#define GUObject_Offset 0xd741460
#define GNativeAndroidApp_Offset 0xd4a4778
////==========================================================================================================//
int screenWidth = -1, glWidth, screenHeight = -1, glHeight;
float density = -1;
uintptr_t UE4;
////==========================================================================================================//
float screenSizeX = 0;
float screenSizeY = 0;
bool ESP[20];
bool head;

////==========================================================================================================//

struct sRegion
{
	uintptr_t start, end;
};

std::vector<sRegion> trapRegions;

bool isEqual(std::string s1, const char* check) {
    std::string s2(check);
    return (s1 == s2);
}

bool isObjectInvalid(UObject *obj)
{
	if (!Tools::IsPtrValid(obj))
	{
		return true;
	}
	if (!Tools::IsPtrValid(obj->ClassPrivate))
	{
		return true;
	}
	if (obj->InternalIndex <= 0)
	{
		return true;
	}
	if (obj->NamePrivate.ComparisonIndex <= 0)
	{
		return true;
	}
	if ((uintptr_t)(obj) % sizeof(uintptr_t) != 0x0 && (uintptr_t)(obj) % sizeof(uintptr_t) != 0x4)
	{
		return true;
	}
	if (std::any_of(trapRegions.begin(), trapRegions.end(), [obj](sRegion region) { return ((uintptr_t)obj) >= region.start && ((uintptr_t)obj) <= region.end; }) ||
		std::any_of(trapRegions.begin(), trapRegions.end(), [obj](sRegion region) { return ((uintptr_t)obj->ClassPrivate) >= region.start && ((uintptr_t)obj->ClassPrivate) <= region.end; }))
	{
		return true;
	}
	return false;
}
////==========================================================================================================//
UWorld *GEWorld;
int GWorldNum = 0;
TUObjectArray gobjects;
UWorld *GetFullWorld()
{
    if(GWorldNum == 0) {
        gobjects = UObject::GUObjectArray->ObjObjects;
        for (int i=0; i< gobjects.Num(); i++)
            if (auto obj = gobjects.GetByIndex(i)) {
                if(obj->IsA(UEngine::StaticClass())) {
                    auto GEngine = (UEngine *) obj;
                    if(GEngine) {
                        auto ViewPort = GEngine->GameViewport;
                        if (ViewPort)
                        {
                            GEWorld = ViewPort->World;
                            GWorldNum = i;
                            return ViewPort->World;
                        }
                    }
                }
            }
    }else {
        auto GEngine = (UEngine *) (gobjects.GetByIndex(GWorldNum));
        if(GEngine) {
            auto ViewPort = GEngine->GameViewport;
            if(ViewPort) {
                GEWorld = ViewPort->World;
                return ViewPort->World;
            }
        }
    }
    return 0;
}

static UGameViewportClient *GameViewport = 0;
UGameViewportClient *GetGameViewport() {
    while (!GameViewport) {
        GameViewport = UObject::FindObject<UGameViewportClient>("GameViewportClient Transient.UAEGameEngine_1.GameViewportClient_1");
        sleep(1);
    }
    if (GameViewport) {
        return GameViewport;
    }
    return 0;
}

std::vector<AActor *> getActors()
{
    auto World = GetFullWorld();
    if (!World)
        return std::vector<AActor *>();
    auto PersistentLevel = World->PersistentLevel;
    if (!PersistentLevel)
        return std::vector<AActor *>();
    auto Actors = *(TArray<AActor *> *)((uintptr_t)PersistentLevel + Actors_Offset);
    std::vector<AActor *> actors;
    for (int i = 0; i < Actors.Num(); i++)
    {
        auto Actor = Actors[i];
        if (Actor)
        {
            actors.push_back(Actor);
        }
    }
    return actors;
}


////==========================================================================================================//
const char *GetVehicleName(ASTExtraVehicleBase *Vehicle) {
    switch (Vehicle->VehicleShapeType) {
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Motorbike:
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Motorbike_SideCart:
            return "Motorbike";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Dacia:
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_HeavyDacia:
            return "Dacia";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_MiniBus:
            return "Mini Bus";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_PickUp:
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_PickUp01:
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_HeavyPickup:
            return "Pick Up";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Buggy:
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_HeavyBuggy:
            return "Buggy";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_UAZ:
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_UAZ01:
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_UAZ02:
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_UAZ03:
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_HeavyUAZ:
            return "UAZ";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_PG117:
            return "PG117";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Aquarail:
            return "Aquarail";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Mirado:
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Mirado01:
            return "Mirado";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Rony:
            return "Rony";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Scooter:
            return "Scooter";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_SnowMobile:
            return "Snow Mobile";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_TukTukTuk:
            return "Tuk Tuk";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_SnowBike:
            return "Snow Bike";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Surfboard:
            return "Surf Board";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Snowboard:
            return "Snow Board";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Amphibious:
            return "Amphibious";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_LadaNiva:
            return "Lada Niva";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_UAV:
            return "UAV";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_MegaDrop:
            return "Mega Drop";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Lamborghini:
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_Lamborghini01:
            return "Lamborghini";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_GoldMirado:
            return "Gold Mirado";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_BigFoot:
            return "Big Foot";
            break;
        case ESTExtraVehicleShapeType::ESTExtraVehicleShapeType__VST_HeavyUH60:
            return "UH60";
            break;
        default:
            return "Vehicle";
            break;
    }
    return "Vehicle";
}
////==========================================================================================================//

TNameEntryArray *GetGNames()
{
	return ((TNameEntryArray * (*)()) (UE4 + GNames_Offset))();
}

template <class T>
void GetAllActors(std::vector<T *> &Actors)
{
	UGameplayStatics *gGameplayStatics = (UGameplayStatics *)gGameplayStatics->StaticClass();
	auto GWorld = GetFullWorld();
	if (GWorld)
	{
		TArray<AActor *> Actors2;
		gGameplayStatics->GetAllActorsOfClass((UObject *)GWorld, T::StaticClass(), &Actors2);
		for (int i = 0; i < Actors2.Num(); i++)
		{
			Actors.push_back((T *)Actors2[i]);
		}
	}
}
////==========================================================================================================//
FVector GetBoneLocationByName(ASTExtraPlayerCharacter *Actor, const char *BoneName)
{
	return Actor->GetBonePos(BoneName, FVector());
}
ASTExtraPlayerCharacter *g_LocalPlayer=0;
ASTExtraPlayerController *g_PlayerController =0;
////==========================================================================================================//
void *LoadFont(void *)
{
	while (!tslFontUI || !robotoTinyFont)
	{
		tslFontUI = UObject::FindObject<UFont>("Font Roboto.Roboto");
		robotoTinyFont = UObject::FindObject<UFont>("Font RobotoDistanceField.RobotoDistanceField");
		sleep(1);
	}
	return 0;
}
////==========================================================================================================//
void NekoHook(FRotator &angles) {
    if (angles.Pitch > 180)
        angles.Pitch -= 360;
    if (angles.Pitch < -180)
        angles.Pitch += 360;

    if (angles.Pitch < -75.f)
        angles.Pitch = -75.f;
    else if (angles.Pitch > 75.f)
        angles.Pitch = 75.f;

    while (angles.Yaw < -180.0f)
        angles.Yaw += 360.0f;
    while (angles.Yaw > 180.0f)
        angles.Yaw -= 360.0f;
}
void NekoHook(float *angles) {
    if (angles[0] > 180)
        angles[0] -= 360;
    if (angles[0] < -180)
        angles[0] += 360;

    if (angles[0] < -75.f)
        angles[0] = -75.f;
    else if (angles[0] > 75.f)
        angles[0] = 75.f;

    while (angles[1] < -180.0f)
        angles[1] += 360.0f;
    while (angles[1] > 180.0f)
        angles[1] -= 360.0f;
}

void NekoHook(FVector2D angles) {
    if (angles.X > 180)
        angles.X -= 360;
    if (angles.X < -180)
        angles.X += 360;

    if (angles.X < -75.f)
        angles.X = -75.f;
    else if (angles.X > 75.f)
        angles.X = 75.f;

    while (angles.Y < -180.0f)
        angles.Y += 360.0f;
    while (angles.Y > 180.0f)
        angles.Y -= 360.0f;
}
////==========================================================================================================//
#define W2S(w, s) UGameplayStatics::ProjectWorldToScreen(g_PlayerController, w, true, s)

float FOVsize = 200.f;

bool isInsideFOV(int x, int y) {
    int circle_x = screenWidth / 2;
    int circle_y = screenHeight / 2;
    int rad = (int) FOVsize;
    return (x - circle_x) * (x - circle_x) + (y - circle_y) * (y - circle_y) <= rad * rad;
}

auto GetTargetForAim()
{
    ASTExtraPlayerCharacter *result = 0;
	float max = std::numeric_limits<float>::infinity();
	auto Actors = getActors();
	
	auto localPlayer = g_LocalPlayer;
    auto localPlayerController = g_PlayerController;
    
	if (localPlayer)
	{
		for (auto Actor : Actors)
		{
			if (isObjectInvalid(Actor))
				continue;
			if (Actor->IsA(ASTExtraPlayerCharacter::StaticClass()))
			{
				auto Player = (ASTExtraPlayerCharacter *)Actor;
                float dist = localPlayer->GetDistanceTo(Player) / 100.0f;    
                   if (dist > 150.f)
                    	continue;
                    
                	if (Player->PlayerKey == localPlayer->PlayerKey)
                    	continue;

                	if (Player->TeamID == localPlayer->TeamID)
                    	continue;

                	if (Player->bDead)
                    	continue;

                	if (Player->Health == 0.0f)
                		continue;

                	if (Player->bEnsure)
                		continue;
					if (!localPlayerController->LineOfSightTo(Player, {0, 0, 0}, true))
                    	continue;

                   auto Root = Player->GetBonePos("Root", {});
                   auto Head = Player->GetBonePos("Head", {});

                   FVector2D RootSc, HeadSc;
                     if (W2S(Root, &RootSc) && W2S(Head, &HeadSc))
                      {
                         float height = abs(HeadSc.Y - RootSc.Y);
                         float width = height * 0.65f;
                        FVector middlePoint = {HeadSc.X + (width / 2), HeadSc.Y + (height / 2),0};
                        if ((middlePoint.X >= 0 && middlePoint.X <= screenWidth) && (middlePoint.Y >= 0 && middlePoint.Y <= screenHeight))
                        {
                        FVector2D v2Middle = FVector2D((float)(screenWidth / 2), (float)(screenHeight / 2));
                        FVector2D v2Loc = FVector2D(middlePoint.X, middlePoint.Y);
                        if(isInsideFOV((int)middlePoint.X, (int)middlePoint.Y)) {
                           float dist = FVector2D::Distance(v2Middle, v2Loc);
                           if (dist < max) {
                                max = dist;
                                result = Player;
                               }                           
                            }
                        }
                    }
                }                
            }
        }
    return result;
}
////==========================================================================================================//
void RenderESP(UCanvas* Canvas, int ScreenWidth, int ScreenHeight)
{
	if(fultaj){

	tslFontUI->LegacyFontSize = 15;
    DrawOutlinedTextFPS(Canvas, FString(DevName), {(float)screenWidth / 7, 70}, COLOR_TEXT, COLOR_IN, true);

	tslFontUI->LegacyFontSize = 15;
	DrawText(Canvas, FString(Version.c_str()), { (float)screenWidth /10 + screenWidth/1.3f, 680 }, COLOR_GREEN, COLOR_BLACK,true);

	ASTExtraPlayerCharacter *localPlayer = 0;
	ASTExtraPlayerController *localPlayerController = 0;

	screenSizeX = ANativeWindow_getWidth(g_App->window);
    screenSizeY = ANativeWindow_getHeight(g_App->window);

	screenWidth = ScreenWidth;
    screenHeight = ScreenHeight;

    		auto Actors = getActors();
			int totalEnemies = 0, totalBots = 0;
    		UGameplayStatics *gGameplayStatics = (UGameplayStatics *)UGameplayStatics::StaticClass();
    		auto GWorld = GetFullWorld();
    		if (GWorld)
    		{
    			UNetDriver *NetDriver = GWorld->NetDriver;
    			if (NetDriver)
    			{
    				UNetConnection *ServerConnection = NetDriver->ServerConnection;
    			if (ServerConnection)
    			{
    				localPlayerController = (ASTExtraPlayerController *)ServerConnection->PlayerController;
    			}
    		}
		
    	    if (localPlayerController) {
    			std::vector<ASTExtraPlayerCharacter *> PlayerCharacter;				
    			GetAllActors(PlayerCharacter);
    		for (auto actor = PlayerCharacter.begin();
    			actor != PlayerCharacter.end(); actor++) {
    		     auto Actor = *actor;
    		if (Actor->PlayerKey ==((ASTExtraPlayerController *) localPlayerController)->PlayerKey) {
    			 localPlayer = Actor;
        	   	 break;
        		}
    	    }

            if (localPlayer) {
				CrashFixer = false;
			   if (ESP[7]) {
				DrawCircle(Canvas, (screenWidth / 2), (screenHeight / 2), FOVsize, 100, COLOR_TEXT);
			    ASTExtraPlayerCharacter *Target = GetTargetForAim();
				if (Target) {
                 if (localPlayer->bIsWeaponFiring) {
					 FVector targetAimPos = Target->GetBonePos("Head", {});
			         auto WeaponManagerComponent = localPlayer->WeaponManagerComponent;
                     if (WeaponManagerComponent)
                     {
                        auto propSlot = WeaponManagerComponent->GetCurrentUsingPropSlot();
                        if ((int)propSlot.GetValue() >= 1 && (int)propSlot.GetValue() <= 3)
                        {
                          auto CurrentWeaponReplicated = (ASTExtraShootWeapon *)WeaponManagerComponent->CurrentWeaponReplicated;
                          if (CurrentWeaponReplicated)
                          {
                                auto ShootWeaponComponent = CurrentWeaponReplicated->ShootWeaponComponent;
                              if (ShootWeaponComponent)
                              {
                                UShootWeaponEntity *ShootWeaponEntityComponent = ShootWeaponComponent->ShootWeaponEntityComponent;
                              if (ShootWeaponEntityComponent)
                              {
                                 ASTExtraVehicleBase *CurrentVehicle = Target->CurrentVehicle;
                                 float dist = localPlayer->GetDistanceTo(Target);
                                 auto timeToTravel = dist / ShootWeaponEntityComponent->BulletRange;
                              if (CurrentVehicle)
                              {
                                 FVector LinearVelocity = CurrentVehicle->ReplicatedMovement.LinearVelocity;
                                 targetAimPos = UKismetMathLibrary::Add_VectorVector(targetAimPos, UKismetMathLibrary::Multiply_VectorFloat(LinearVelocity, timeToTravel));
                               }else{
                                 FVector Velocity = Target->GetVelocity();
                                 targetAimPos = UKismetMathLibrary::Add_VectorVector(targetAimPos, UKismetMathLibrary::Multiply_VectorFloat(Velocity, timeToTravel));
                               }
                            
                               if (localPlayer->bIsWeaponFiring)
                               {
                                 float dist = localPlayer->GetDistanceTo(Target) / 100.f;
                                 targetAimPos.Z -= dist * 1.2;
                                }
                            
                                     FVector fDir = UKismetMathLibrary::Subtract_VectorVector(targetAimPos, g_PlayerController->PlayerCameraManager->CameraCache.POV.Location);
                                     FRotator Yaptr = UKismetMathLibrary::Conv_VectorToRotator(fDir);
                                
                                     FRotator CpYaT = g_PlayerController->PlayerCameraManager->CameraCache.POV.Rotation;
                                
                                     Yaptr.Pitch -= CpYaT.Pitch;
                                     Yaptr.Yaw -= CpYaT.Yaw;
                                     Yaptr.Roll = 0.f;
                                     NekoHook(Yaptr);
                                
                                     CpYaT.Pitch += Yaptr.Pitch / 2.4; // Aim X Speed Make Float : Xs
                                     CpYaT.Yaw += Yaptr.Yaw / 2.4; // Aim Y Speed Make Float : Ys
                                     CpYaT.Roll = 0.f;
                                
                                      g_PlayerController->ControlRotation=CpYaT;
                                      }
                                   }
                                }
                             }
                          }
                      }
                   }
               }
			   
	if (ESP[8]){
uintptr_t Mesh = *(uintptr_t *)((uintptr_t)localPlayer + 0x498);
if (Mesh != 0)
{
    uintptr_t Skeletal = *(uintptr_t *)(Mesh + 0x868);
    if (Skeletal != 0)
    {
        uintptr_t Asset = *(uintptr_t *)(Skeletal + 0x138);
        if (Asset != 0)
        {
            uintptr_t ArraySkeletal = *(uintptr_t *)(Asset + 0x38);
            if (ArraySkeletal != 0)
            {
                uintptr_t SkeletalBodySetup = *(uintptr_t *)(ArraySkeletal + sizeof(uintptr_t) * 14);

                if (SkeletalBodySetup != 0)
                {
                    uintptr_t BoxElems = *(uintptr_t *)(SkeletalBodySetup + 0x38);
                    if (BoxElems != 0)
                    {
                      *(float *)(BoxElems + 0x88) = 90.0f;
                      *(float *)(BoxElems + 0x8c) = 95.0f;
                      *(float *)(BoxElems + 0x90) = 99.0f;
                    }
                    }
                    }
                    }
                    }
                    }
                    }

////==========================================================================================================//               
               
            	int totalEnemies = 0, totalBots = 0;
            	std::vector<ASTExtraPlayerCharacter *> PlayerCharacter;
                GetAllActors(PlayerCharacter);
			    for (auto actor = PlayerCharacter.begin(); actor != PlayerCharacter.end(); actor++)
			    {

                auto Player = *actor;
                if (Player->PlayerKey == localPlayer->PlayerKey)
                	continue;
                if (Player->TeamID == localPlayer->TeamID)
                	continue;
                if (Player->bDead)
                	continue;
                if (Player->bHidden)
                	continue;
                                                                
                if (!Player->RootComponent)
                	continue;
                                        
                 if (ESP[1]) {
                if (Player->bEnsure)
                      continue;
                 }
                	
                if (Player->bEnsure)
                 totalBots++;
                 else totalEnemies++;
                        
                float Distance = localPlayer->GetDistanceTo(Player) / 100.0f;
                if (Distance > 500)
                	continue;            
                                                
                FVector HeadPos = GetBoneLocationByName(Player,"Head");
                FVector2D HeadPosSC;
                FVector RootPos = GetBoneLocationByName(Player,"Root");
                FVector2D RootPosSC;
                FVector Root = GetBoneLocationByName(Player,"Root");
                FVector Spin = GetBoneLocationByName(Player,"pelvis");
                FVector Spin2 = GetBoneLocationByName(Player,"spine_03");
                FVector pelvis = GetBoneLocationByName(Player,"pelvis");
                FVector2D pelvisPoSC;
                FVector upper_r = GetBoneLocationByName(Player,"upperarm_r");
                FVector2D upper_rPoSC;
                FVector lowerarm_r = GetBoneLocationByName(Player,"lowerarm_r");
                FVector2D lowerarm_rPoSC;
                FVector lowerarm_l = GetBoneLocationByName(Player,"lowerarm_l");
                FVector2D lowerarm_lSC;
                FVector hand_r = GetBoneLocationByName(Player,"hand_r");
                FVector2D hand_rPoSC;
                FVector upper_l = GetBoneLocationByName(Player,"upperarm_l");
                FVector2D upper_lPoSC;
                FVector hand_l = GetBoneLocationByName(Player,"hand_l");
                FVector2D hand_lPoSC;
                FVector thigh_l = GetBoneLocationByName(Player,"thigh_l");
                FVector2D thigh_lPoSC;
                FVector calf_l = GetBoneLocationByName(Player,"calf_l");
                FVector2D calf_lPoSC;
                FVector foot_l = GetBoneLocationByName(Player,"foot_l");
                FVector2D foot_lPoSC;
                FVector thigh_r = GetBoneLocationByName(Player,"thigh_r");
                FVector2D thigh_rPoSC;
                FVector calf_r = GetBoneLocationByName(Player,"calf_r");
                FVector2D calf_rPoSC;
                FVector foot_r = GetBoneLocationByName(Player,"foot_r");
                FVector2D foot_rPoSC;
                FVector neck_01 = GetBoneLocationByName(Player,"neck_01");
                FVector2D neck_01PoSC;
                FVector spine_01 = GetBoneLocationByName(Player,"spine_01");
                FVector2D spine_01PoSC;
                FVector spine_02 = GetBoneLocationByName(Player,"spine_02");
                FVector2D spine_02PoSC;
                FVector spine_03 = GetBoneLocationByName(Player,"spine_03");
                FVector2D spine_03PoSC;               
////==========================================================================================================//
                if (gGameplayStatics->ProjectWorldToScreen(g_PlayerController, HeadPos, false, &HeadPosSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, lowerarm_l, false, &lowerarm_lSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, upper_r, false, &upper_rPoSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, upper_l, false, &upper_lPoSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, lowerarm_r, false, &lowerarm_rPoSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, hand_r, false, &hand_rPoSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, hand_l, false, &hand_lPoSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, thigh_l, false, &thigh_lPoSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, calf_l, false, &calf_lPoSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, foot_l, false, &foot_lPoSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, thigh_r, false, &thigh_rPoSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, calf_r, false, &calf_rPoSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, foot_r, false, &foot_rPoSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, neck_01, false, &neck_01PoSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, pelvis, false, &pelvisPoSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, RootPos, false, &RootPosSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, spine_01, false, &spine_01PoSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, spine_02, false, &spine_02PoSC) &&
                    gGameplayStatics->ProjectWorldToScreen(g_PlayerController, spine_03, false, &spine_03PoSC)) {
////==========================================================================================================//
                if (ESP[3]) {
                    FVector2D head = {HeadPosSC.X, HeadPosSC.Y};//head
                    FVector2D 左肩膀 = {upper_lPoSC.X, upper_lPoSC.Y};//左上肩
                    FVector2D 右肩膀 = {upper_rPoSC.X, upper_rPoSC.Y};//右上肩
                    FVector2D 右小臂 = {lowerarm_rPoSC.X, lowerarm_rPoSC.Y};//右小臂
                    FVector2D 左小臂 = {lowerarm_lSC.X, lowerarm_lSC.Y};//左小臂
                    FVector2D 右大腿 = {thigh_rPoSC.X, thigh_rPoSC.Y};//右大腿
                    FVector2D 左大腿 = {thigh_lPoSC.X, thigh_lPoSC.Y};//左大腿
                    FVector2D 左小腿 = {calf_lPoSC.X, calf_lPoSC.Y};//左小腿
                    FVector2D 右小腿 = {calf_rPoSC.X, calf_rPoSC.Y};//右小腿
                    FVector2D 腰部 = {spine_01PoSC.X, spine_01PoSC.Y};//右小腿
                    FVector2D 腰部3 = {spine_03PoSC.X, spine_03PoSC.Y};//右小腿
                    FVector2D Neck = {neck_01PoSC.X, neck_01PoSC.Y};//右小腿
                    FVector2D 腰部2 = {spine_02PoSC.X, spine_02PoSC.Y};//右小腿
                    float boxWidth = 7.f - Distance * 0.03;
                    
                    if(!g_PlayerController->LineOfSightTo(g_PlayerController->PlayerCameraManager,Player->GetBonePos("Head", {0, 0, 0}), false))//头
                    {
                    DrawLine(Canvas,head,HeadPosSC,THIKNESS,COLOR_BONE);
                    } else {
                    DrawLine(Canvas,head,HeadPosSC,THIKNESS,COLOR_BONE_VIS);
                    }
                    if(!g_PlayerController->LineOfSightTo(g_PlayerController->/*落*/PlayerCameraManager,Player->GetBonePos("neck_01", {0, 0, 0}), false))//Neck
                    {
                    DrawLine(Canvas,Neck,HeadPosSC,THIKNESS,COLOR_BONE);
                    DrawLine(Canvas,Neck,spine_03PoSC,THIKNESS,COLOR_BONE);
                    } else {
                    DrawLine(Canvas,Neck,HeadPosSC,THIKNESS,COLOR_BONE_VIS);
                    DrawLine(Canvas,Neck,spine_03PoSC,THIKNESS,COLOR_BONE_VIS);
                    }
                    if(!g_PlayerController->LineOfSightTo(g_PlayerController->PlayerCameraManager,Player->GetBonePos("upperarm_r", {0, 0, 0}), false))//上面的肩膀右
                    {
                    DrawLine(Canvas,右肩膀,neck_01PoSC,THIKNESS,COLOR_BONE);
                    DrawLine(Canvas,右肩膀,hand_rPoSC,THIKNESS,COLOR_BONE);
                    }else {
                    DrawLine(Canvas,右肩膀,neck_01PoSC,THIKNESS,COLOR_BONE_VIS);
                    DrawLine(Canvas,右肩膀,hand_rPoSC,THIKNESS,COLOR_BONE_VIS);
                    }
                    
                    if(!g_PlayerController->LineOfSightTo(g_PlayerController->PlayerCameraManager,Player->GetBonePos("upperarm_l", {0, 0, 0}), false))//上面的肩膀左
                    {
                    DrawLine(Canvas,左肩膀,lowerarm_lSC,THIKNESS,COLOR_BONE);
                    DrawLine(Canvas,左肩膀,neck_01PoSC,THIKNESS,COLOR_BONE);
                    } else {
                    DrawLine(Canvas,左肩膀,lowerarm_lSC,THIKNESS,COLOR_BONE_VIS);
                    DrawLine(Canvas,左肩膀,neck_01PoSC,THIKNESS,COLOR_BONE_VIS);
                    }
                    
                    if(!g_PlayerController->LineOfSightTo(g_PlayerController->PlayerCameraManager,Player->GetBonePos("lowerarm_r", {0, 0, 0}), false))//上面的手臂右
                    {
                    DrawLine(Canvas,右小臂,lowerarm_rPoSC,THIKNESS,COLOR_BONE);
                    } else {
                    DrawLine(Canvas,右小臂,lowerarm_rPoSC,THIKNESS,COLOR_BONE_VIS);
                    }
                    if(!g_PlayerController->LineOfSightTo(g_PlayerController->PlayerCameraManager,Player->GetBonePos("lowerarm_l", {0, 0, 0}), false))//上面的手臂左
                    {
                    DrawLine(Canvas,左小臂,lowerarm_lSC,THIKNESS,COLOR_BONE);
                    DrawLine(Canvas,左小臂,hand_lPoSC,THIKNESS,COLOR_BONE);
                    } else {
                    DrawLine(Canvas,左小臂,lowerarm_lSC,THIKNESS,COLOR_BONE_VIS);
                    DrawLine(Canvas,左小臂,hand_lPoSC,THIKNESS,COLOR_BONE_VIS);
                    }
                    if(!g_PlayerController->LineOfSightTo(g_PlayerController->PlayerCameraManager,Player->GetBonePos("spine_03", {0, 0, 0}), false))//脊柱3
                    {
                    DrawLine(Canvas,腰部3,spine_02PoSC,THIKNESS,COLOR_BONE);
                    } else {
                    DrawLine(Canvas,腰部3,spine_02PoSC,THIKNESS,COLOR_BONE_VIS);
                    }
                    if(!g_PlayerController->LineOfSightTo(g_PlayerController->PlayerCameraManager,Player->GetBonePos("spine_02", {0, 0, 0}), false))//脊柱2
                    {
                    DrawLine(Canvas,腰部2,spine_01PoSC,THIKNESS,COLOR_BONE);
                    } else {
                    DrawLine(Canvas,腰部2,spine_01PoSC,THIKNESS,COLOR_BONE_VIS);
                    }
                    if(!g_PlayerController->LineOfSightTo(g_PlayerController->PlayerCameraManager,Player->GetBonePos("spine_01", {0, 0, 0}), false))//脊柱1
                    {
                    DrawLine(Canvas,腰部,pelvisPoSC,THIKNESS,COLOR_BONE);
                    } else {
                    DrawLine(Canvas,腰部,pelvisPoSC,THIKNESS,COLOR_BONE_VIS);
                    }
                    
                    if(!g_PlayerController->LineOfSightTo(g_PlayerController->PlayerCameraManager,Player->GetBonePos("thigh_l", {0, 0, 0}), false))//大腿左
                    {
                    DrawLine(Canvas,左大腿,calf_lPoSC,THIKNESS,COLOR_BONE);
                    } else {
                    DrawLine(Canvas,左大腿,calf_lPoSC,THIKNESS,COLOR_BONE_VIS);
                    }
                    if(!g_PlayerController->LineOfSightTo(g_PlayerController->PlayerCameraManager,Player->GetBonePos("thigh_r", {0, 0, 0}), false))//大腿右
                    {
                    DrawLine(Canvas,右大腿,calf_rPoSC,THIKNESS,COLOR_BONE);
                    DrawLine(Canvas,右大腿,thigh_lPoSC,THIKNESS,COLOR_BONE);
                    } else {
                    DrawLine(Canvas,右大腿,calf_rPoSC,THIKNESS,COLOR_BONE_VIS);
                    DrawLine(Canvas,右大腿,thigh_lPoSC,THIKNESS,COLOR_BONE_VIS);
                    }
                    if(!g_PlayerController->LineOfSightTo(g_PlayerController->PlayerCameraManager,Player->GetBonePos("calf_l", {0, 0, 0}), false))//小腿左
                    {
                    DrawLine(Canvas,左小腿,foot_lPoSC,THIKNESS,COLOR_BONE);
                    } else{
                    DrawLine(Canvas,左小腿,foot_lPoSC,THIKNESS,COLOR_BONE_VIS);
                    }
                    if(!g_PlayerController->LineOfSightTo(g_PlayerController->PlayerCameraManager,Player->GetBonePos("calf_r", {0, 0, 0}), false))//小腿右
                    {
                    DrawLine(Canvas,右小腿,calf_rPoSC,THIKNESS,COLOR_BONE);
                    DrawLine(Canvas,右小腿,foot_rPoSC,THIKNESS,COLOR_BONE);
                    } else {
                    DrawLine(Canvas,右小腿,foot_rPoSC,THIKNESS,COLOR_BONE_VIS);
                    DrawLine(Canvas,右小腿,calf_rPoSC,THIKNESS,COLOR_BONE_VIS);
                    }
                }            
                                             
////==========================================================================================================//
bool IsVisible = g_PlayerController->LineOfSightTo(Player, {0,0,0}, true);
if (ESP[2]){
if(Player->bEnsure){
if (IsVisible)
DrawLine(Canvas, {static_cast<float>(screenWidth / 2), 70}, FVector2D(HeadPosSC.X, HeadPosSC.Y - 62), THIKNESS,COLOR_BONE_VIS);
else
DrawLine(Canvas, {static_cast<float>(screenWidth / 2), 70}, FVector2D(HeadPosSC.X, HeadPosSC.Y - 62), THIKNESS,COLOR_BONE);
}else{
if (IsVisible)
DrawLine(Canvas, {static_cast<float>(screenWidth / 2), 70}, FVector2D(HeadPosSC.X, HeadPosSC.Y - 62), THIKNESS,COLOR_BONE_VIS);
else
DrawLine(Canvas, {static_cast<float>(screenWidth / 2), 70}, FVector2D(HeadPosSC.X, HeadPosSC.Y - 62), THIKNESS,COLOR_BONE);
}
}

////==========================================================================================================//                             
        

if (ESP[5]) {
    FVector BelowRoot = Root;
    BelowRoot.Z -= 55.f; // Adjusted to be at the player's feet
    FVector2D BelowRootSc;
    
    if (gGameplayStatics->ProjectWorldToScreen(localPlayerController, BelowRoot, false, &BelowRootSc)) {
        std::wstring wsName;   // Player Name or "Bot"
        std::wstring wsDist;   // Distance
        
        // Check if the player is a bot or a real player
        if (Player->bEnsure) {
            wsName = L"Bot";
        } else {
            wsName = Player->PlayerName.ToWString();
        }

        // Format the distance text
        wsDist = std::to_wstring((int)Distance) + L"M"; // Adding "m" for meters

        // Adjust font size based on distance
        tslFontUI->LegacyFontSize = max(5, 12 - (int)(Distance / 80));

        // Draw the player's name below the feet
        DrawOutlinedTextFPS(Canvas, 
            FString(wsName), 
            FVector2D(BelowRootSc.X, BelowRootSc.Y + 10),  // Name moved down by 10 pixels
            COLOR_TEXT, COLOR_IN, true);

        // Draw the distance further below the name
        DrawOutlinedTextFPS(Canvas, 
            FString(wsDist), 
            FVector2D(BelowRootSc.X, BelowRootSc.Y + 30), // Increased offset (was 20, now 30)
            COLOR_TEXT, COLOR_IN, true);
    }
}

////==========================================================================================================//
     if (ESP[4]) {
      
    float CurHP = std::max(0.f, std::min(Player->Health, Player->HealthMax));
    float MaxHP = Player->HealthMax;
    float HealthPercentage = CurHP / MaxHP;

    // Update the health color logic
    FVector AboveHead = Player->GetHeadLocation(true);
    AboveHead.Z += 25.f;

    FVector2D AboveHeadSc;
    if (gGameplayStatics->ProjectWorldToScreen(localPlayerController, AboveHead, false, &AboveHeadSc)) {
        float mWidth = 60.0f;
        float mHeight = 6.0f;

        AboveHeadSc.X -= mWidth / 2;
        AboveHeadSc.Y -= mHeight * 1.5f;

        // Draw the health bar
        DrawFilledRect(Canvas, {AboveHeadSc.X, AboveHeadSc.Y}, (CurHP * mWidth / MaxHP), mHeight, COLOR_HEALTH);

        // Draw the black border around the health bar
        DrawRectangle(Canvas, {AboveHeadSc.X, AboveHeadSc.Y}, mWidth, mHeight, 1.5f, COLOR_IN); // Black border
		}
	  }
	}
  }

						if (ESP[6]) {                                       
    std::vector<ASTExtraVehicleBase*> VehicleBase;
    GetAllActors(VehicleBase);
    
    for (auto actor = VehicleBase.begin(); actor != VehicleBase.end(); actor++) {
        auto Vehicle = *actor;
        if (!Vehicle->Mesh)
            continue;
        if (!Vehicle->RootComponent)
            continue;
        
        float Distance = Vehicle->GetDistanceTo(localPlayer) / 100.f;
        if (Distance > 500)
            continue;
        
        FVector2D VehiclePos;
        if (gGameplayStatics->ProjectWorldToScreen(g_PlayerController, Vehicle->RootComponent->RelativeLocation, false, &VehiclePos)) {
            std::string vehicleText = GetVehicleName(Vehicle); // Get only vehicle name
            std::string distanceText = std::to_string((int)Distance) + "M"; // Vehicle distance
           
            // Adjust font size based on distance
            tslFontUI->LegacyFontSize = max(6, 12 - (int)(Distance / 80));
            
            // Draw vehicle distance (on top)
            DrawOutlinedTextFPS(Canvas, FString(distanceText), 
                FVector2D(VehiclePos.X, VehiclePos.Y +20), 
                COLOR_TEXT, COLOR_IN, true);

            // Draw vehicle name (below distance)
            DrawOutlinedTextFPS(Canvas, FString(vehicleText), 
                FVector2D(VehiclePos.X, VehiclePos.Y), // Move name down by 15 pixels
                COLOR_TEXT, COLOR_IN, true);
        }
    }
}
////==========================================================================================================//
g_LocalPlayer = localPlayer;
g_PlayerController = localPlayerController;
    std::string playerInfo;
    if (totalEnemies + totalBots > 0) {
    playerInfo = "[ ";
    playerInfo += std::to_string(totalEnemies + totalBots);
    playerInfo += " ]";
	tslFontUI->LegacyFontSize = 18;
	DrawOutlinedTextFPS(Canvas, FString(playerInfo), {(float)screenWidth / 2, 70}, COLOR_TEXT, COLOR_IN, true);
	}else{
    playerInfo = "[ Clear ]";
    tslFontUI->LegacyFontSize = 18;
    DrawOutlinedTextFPS(Canvas, FString(playerInfo), {(float)screenWidth / 2, 70}, COLOR_TEXT, COLOR_IN, true);
	}
   }
  }
 }
}

////==========================================================================================================//

if (fultaj){
	tslFontUI->LegacyFontSize = 20;
    DrawOutlinedTextFPS(Canvas, FString(loadername), {(float)screenWidth / 2, 30}, COLOR_TEXT, COLOR_IN, true);

	}else{

	std::string loginkeyerrot = oxorany("Wrong Key");
	tslFontUI->LegacyFontSize = 18;
    DrawOutlinedTextFPS(Canvas, FString(loginkeyerrot), {(float)screenWidth / 2, 10}, COLOR_TEXT, COLOR_IN, true);

	}

////==========================================================================================================//	

EagleGUI::SetupCanvas(Canvas,tslFontUI);
static FVector2D WindowPos = {300,200};
static bool IsOpen = true;
static float TempValue = 0;        
tslFontUI->LegacyFontSize = 13;
 }

////==========================================================================================================// 
#include <unistd.h>
#include <sys/mman.h>

using PostRenderFn = void(*)(UGameViewportClient*, UCanvas*);
PostRenderFn orig_fultajcheats = nullptr;

void hook_fultajcheats(UGameViewportClient* ViewportClient, UCanvas* Canvas) {
    if (Canvas) {
        RenderESP(Canvas, Canvas->SizeX, Canvas->SizeY);
    }
    if (orig_fultajcheats) {
        orig_fultajcheats(ViewportClient, Canvas);
    }
}

void* GetPostRenderAddr() {
    auto GViewport = GetGameViewport();
    if (!GViewport || !GViewport->VTable) return nullptr;

    return reinterpret_cast<void**>(GViewport->VTable)[132]; // VTable index for PostRender
}

void PostrenderDraw() {
    void* addr = GetPostRenderAddr();
    if (addr) {
        A64HookFunction(addr, (void*)&hook_fultajcheats, (void**)&orig_fultajcheats);
    }
}

#include <list>
#include <vector>
#include <string.h>
#include <pthread.h>
#include <thread>
#include <cstring>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dlfcn.h>
#include "Main/Logger.h"
#include "Main/Utils.h"
#include "Main/Macros.h"
#define _BYTE  uint8_t
#define _WORD  uint16_t
#define _DWORD uint32_t
#define _QWORD uint64_t

void *fultaj_thread(void *) {

    while (!UE4) {
        UE4 = Tools::GetBaseAddress("libUE4.so");
        sleep(1);
    }    
 if (CrashFixer)
 {
PATCH_LIB("libUE4.so","0xBC66600","00 00 80 D2 C0 03 5F D6");
PATCH_LIB("libUE4.so","0xBC66640","00 00 80 D2 C0 03 5F D6");
PATCH_LIB("libUE4.so","0xBC66650","00 00 80 D2 C0 03 5F D6");
 }
    while (!g_App) {
        g_App = *(android_app * *)(UE4 + GNativeAndroidApp_Offset);
        
        sleep(1);
    }

    FName::GNames = GetGNames();
    while (!FName::GNames) {
        FName::GNames = GetGNames();
        sleep(1);
    }

    UObject::GUObjectArray = (FUObjectArray * )(UE4 + GUObject_Offset);

    static bool loadFont = false;
	if (!loadFont)
	{
		pthread_t t;
		pthread_create(&t, 0, LoadFont, 0);
		loadFont = true;
	}

    PostrenderDraw();

    ESP[2] = true;
	ESP[3] = true;
	ESP[4] = true;
	ESP[5] = true;
	ESP[6] = true;
	ESP[8] = true;

    return 0;    
}
__attribute__((constructor)) void _init() {
    pthread_t Ptid;
	pthread_create(&Ptid, 0,fultaj_thread, 0);
	
}

