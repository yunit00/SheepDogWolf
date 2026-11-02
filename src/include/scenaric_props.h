// Property blocks of scenaric object instances as stored in.WAR files.
#ifndef SCENARIC_PROPS_H
#define SCENARIC_PROPS_H

typedef unsigned int u32;

typedef enum ScenaricClassId {
    CLASSID_WOLF = 0,
    CLASSID_SAM = 1,
    CLASSID_DYNAMITE = 2,
    CLASSID_SALAD = 3,
    CLASSID_MAILBOX = 4,
    CLASSID_BALANCE = 5,
    CLASSID_BRIDGE = 6,
    CLASSID_FAN = 7,
    CLASSID_ROBIN = 8,
    CLASSID_PORKY = 9,
    CLASSID_ROCKET = 10,
    CLASSID_SHEEP = 11,
    CLASSID_MISCSTATIC = 12,
    CLASSID_ROCKS = 13,
    CLASSID_BOX = 14,
    CLASSID_SEESAW = 15,
    CLASSID_ROCK = 16,
    CLASSID_TREE_02 = 17,
    CLASSID_CAMERAMANAGER = 18,
    CLASSID_CAMERARESTRICTION = 19,
    CLASSID_FALLINGROCK = 20,
    CLASSID_SIGNPOST = 21,
    CLASSID_DAFFYTRAININGLEVEL = 22,
    CLASSID_GOAL = 23,
    CLASSID_SENSIBLEBUTTON = 24,
    CLASSID_CHECKPOINTMANAGER = 25,
    CLASSID_FALLINGGATE = 26,
    CLASSID_CACTUS = 27,
    CLASSID_TELESCOPE = 28,
    CLASSID_ANVIL = 29,
    CLASSID_PORKYLEVEL01 = 30,
    CLASSID_BIPBIP = 31,
    CLASSID_CINEMATICSMANAGER = 32,
    CLASSID_DAFFYLEVEL01 = 33,
    CLASSID_TVSCENEMANAGER = 34,
    CLASSID_HIDDENROCKS = 35,
    CLASSID_WOODEN_LIFT = 36,
    CLASSID_GOLDENROCK = 37,
    CLASSID_TIMEKEEPER = 38,
    CLASSID_PERFUME = 39,
    CLASSID_WOODENPLATFORM = 40,
    CLASSID_FLUTE = 41,
    CLASSID_ELASTIC = 42,
    CLASSID_BULL = 43,
    CLASSID_SECRETDOOR = 44,
    CLASSID_ELASTICTREE = 45,
    CLASSID_ELASTICHOOK = 46,
    CLASSID_DAFFYLEVEL02 = 47,
    CLASSID_BUSH = 48,
    CLASSID_REDTHING = 49,
    CLASSID_SHARK = 50,
    CLASSID_REDSCARF = 51,
    CLASSID_TRIGGEDSTONE = 52,
    CLASSID_RAFT = 53,
    CLASSID_HEAPOFLEAF = 54,
    CLASSID_DAFFYELF = 55,
    CLASSID_CATAPULT = 56,
    CLASSID_DAFFYMILITARY = 57,
    CLASSID_HAIRDRYER = 58,
    CLASSID_DOORMECHANISM = 59,
    CLASSID_ICECUBE = 60,
    CLASSID_MINEDETECTOR = 61,
    CLASSID_UMBRELLA = 62,
    CLASSID_STUMP = 63,
    CLASSID_DEFUSABLEMINE = 64,
    CLASSID_SMALLROCK = 65,
    CLASSID_MAGNET = 66,
    CLASSID_FISHINGROD = 67,
    CLASSID_MAGNETROD = 68,
    CLASSID_SEAWEED = 69,
    CLASSID_VISIBILITYMANAGER = 70,
    CLASSID_HITSWITCH = 71,
    CLASSID_GEYSERIN = 72,
    CLASSID_GEYSEROUT = 73,
    CLASSID_FOGMANAGER = 74,
    CLASSID_SNOWBALL = 75,
    CLASSID_SNOWYGROUND = 76,
    CLASSID_ICEGROUND = 77,
    CLASSID_SIGNPOSTSIMPLE = 78,
    CLASSID_SLIDINGICECUBE = 79,
    CLASSID_SHEEPCOSTUME = 80,
    CLASSID_WOLFTRAP = 81,
    CLASSID_CRANE = 82,
    CLASSID_FROZENRIVER = 83,
    CLASSID_DRAGON = 84,
    CLASSID_TIMEMACHINECHRONO = 85,
    CLASSID_TIMEMACHINESPHERE = 86,
    CLASSID_GOSSAMER_LEV08 = 87,
    CLASSID_GROUNDMINE = 88,
    CLASSID_SWIRLSIGN = 89,
    CLASSID_ELMER = 90,
    CLASSID_TREESECTION = 91,
    CLASSID_BULLET = 92,
    CLASSID_SEED = 93,
    CLASSID_TREE = 94,
    CLASSID_CANNONBALL = 95,
    CLASSID_PIPE = 96,
    CLASSID_CANONSIMPLE = 97,
    CLASSID_MCARDMANAGER = 98,
    CLASSID_RABBITCOSTUME = 99,
    CLASSID_REMOTECONTROL = 100,
    CLASSID_ROBOT = 101,
    CLASSID_BELL = 102,
    CLASSID_WATCH = 103,
    CLASSID_DAFFYLEVEL09 = 104,
    CLASSID_CANONDUMMY = 105,
    CLASSID_MAPLOCATION = 106,
    CLASSID_BEES = 107,
    CLASSID_HONEYPOT = 108,
    CLASSID_CROCODILELEVEL09 = 109,
    CLASSID_DIAMOND = 110,
    CLASSID_WHEEL = 111,
    CLASSID_WHEELDUMMY = 112,
    CLASSID_GOSSAMER_BOSS = 113,
    CLASSID_SUPERBUTTON = 114,
    CLASSID_ROLLINGCARPET = 115,
    CLASSID_HIVE = 116,
    CLASSID_PIRANHAS = 117,
    CLASSID_RCARPETMOBILE = 118,
    CLASSID_CRUMBLYGROUND = 119,
    CLASSID_GHOST = 120,
    CLASSID_SALADROD = 121,
    CLASSID_CROCODILELEVEL11 = 122,
    CLASSID_MONOLITHE = 123,
    CLASSID_LIGHTSPOT = 124,
    CLASSID_GHOSTCOSTUME = 125,
    CLASSID_JAIL = 126,
    CLASSID_DANCINGGHOST = 127,
    CLASSID_BATTERY = 128,
    CLASSID_KEY = 129,
    CLASSID_PRAYINGGHOST = 130,
    CLASSID_HOOVER = 131,
    CLASSID_INFLATABLESHEEP = 132,
    CLASSID_TRAFFICJAMS = 133,
    CLASSID_CHRONOMETER = 134,
    CLASSID_TRAIN = 135,
    CLASSID_TRAINSTATION = 136,
    CLASSID_AMBIENTSOUNDMANAGER = 137,
    CLASSID_GHOSTHALO = 138,
    CLASSID_BIPBIPLEVEL14 = 139,
    CLASSID_FALLINGGATE2 = 140,
    CLASSID_GOLDENCOINS = 141,
    CLASSID_SAM_PIRATE = 142,
    CLASSID_FLOATINGBOX = 143,
    CLASSID_INSTANTHOOVER = 144,
    CLASSID_INSTANTMARTIAN = 145,
    CLASSID_CANONSHEEP = 146,
    CLASSID_CANNONBALL2 = 147,
    CLASSID_SCENESHEEPPANEL = 148,
    CLASSID_LAZERROBOT = 149,
    CLASSID_LASER = 150,
    CLASSID_CASE = 151,
    CLASSID_SCENE_WHEEL = 152,
    CLASSID_SCENE_ARROW = 153,
    CLASSID_SCENE_SCREEN = 154,
    CLASSID_DAFFYSCENE = 155,
    CLASSID_AUTOMATICDOOR = 156,
    CLASSID_RESIZER = 157,
    CLASSID_BAT = 158,
    CLASSID_FIREFLY = 159,
    CLASSID_BIRD = 160,
    CLASSID_BUTTERFLY = 161,
    CLASSID_FISH = 162,
    CLASSID_PIPE2 = 163,
    CLASSID_LAVA = 164,
    CLASSID_SIGNPOSTANIMATED = 165,
    CLASSID_VOLCANO = 166,
    CLASSID_DANCINGGHOSTMANAGER = 167,
    CLASSID_DOORSMASTER = 168,
    CLASSID_DOORWORLD = 169,
    CLASSID_DOORLEVEL = 170,
    CLASSID_FIREBALL = 171,
    CLASSID_TWIG = 172,
    CLASSID_GEYSERMANGER = 173,
    CLASSID_SIGNTIPS = 174,
    CLASSID_MIRRORMANAGER = 175,
    CLASSID_CREDITSMANAGER = 176,
    CLASSID_WATERMINE = 177,
    CLASSID_ROOK = 178,
    CLASSID_SFXCINEMANAGER = 179,
    CLASSID_CINEMATRIXMODE = 180,
    CLASSID_CRUMBLYPLAT = 181,
    CLASSID_GOSSAMERONDE = 182,
    CLASSID_FACINGCAMERA = 183,
    CLASSID_OBJECTMANAGER = 184,
    CLASSID_HIDDENBUTTON = 185,
    CLASSID_WATERGEYSER = 186,
    CLASSID_DAFFYWHEEL = 187,
    CLASSID_TORCH = 188,
    CLASSID_SAIL = 189,
    CLASSID_BONUSMANAGER = 190,
    CLASSID_LEAF = 191,
    CLASSID_BLACKHOLE = 192,
    CLASSID_MARVIN = 193,
    CLASSID_INSTANTSOCKET = 194,
    CLASSID_CROWD = 195,
    CLASSID_CAMERAMANAGER2 = 196,
    CLASSID_COUNT = 197,
    CLASSID_NONE = 0xffff  // not in the disc header: 'no class' (Scenaric_CreateObject, the inventory selection, CannonBall.hitClassId, the Map lists)
} ScenaricClassId;

// GameRes.h: ids of the resources exported by the level files (WAR_IDO_* objects and boxes, DAV_IDI_* images)
typedef enum GameResId {
    WAR_IDO_ACAISS01 = 1,
    WAR_IDO_AETOIL01 = 2,
    WAR_IDO_AEXPLOS1 = 3,
    WAR_IDO_AEXPLOS2 = 4,
    WAR_IDO_ATESAM01 = 5,
    WAR_IDO_CAMCOLLBOX = 6,
    WAR_IDO_CAMNOCOLLBOX = 7,
    WAR_IDO_CINBBOX = 8,
    WAR_IDO_CINSHEEPBBOX = 9,
    WAR_IDO_DEATHBOX = 10,
    DAV_IDI_I01LVU4_ = 11,
    DAV_IDI_I01LVU5_ = 12,
    DAV_IDI_IDCICONA = 13,
    DAV_IDI_IDCICONB = 14,
    DAV_IDI_IDYICONA = 15,
    DAV_IDI_IDYICONB = 16,
    DAV_IDI_IFUICONA = 17,
    DAV_IDI_IFUICONB = 18,
    DAV_IDI_IGLCADG_ = 19,
    DAV_IDI_IGLCADH_ = 20,
    DAV_IDI_IGLCADP_ = 21,
    DAV_IDI_IGLCERC_ = 22,
    DAV_IDI_IGLCOMBA = 23,
    DAV_IDI_IGLCOMBB = 24,
    DAV_IDI_IGLCRAI1 = 25,
    DAV_IDI_IGLEXITA = 26,
    DAV_IDI_IGLEXITB = 27,
    DAV_IDI_IGLFLEC_ = 28,
    DAV_IDI_IGLFONTE = 29,
    DAV_IDI_IGLHELPA = 30,
    DAV_IDI_IGLHELPB = 31,
    DAV_IDI_IGLNEIG1 = 32,
    DAV_IDI_IGLOMBR1 = 33,
    DAV_IDI_IGLSMOK_ = 34,
    DAV_IDI_IGLTRIA_ = 35,
    DAV_IDI_IGLTYPO_ = 36,
    DAV_IDI_IGLUSEDA = 37,
    DAV_IDI_IGLUSEDB = 38,
    DAV_IDI_IGLVOL0_ = 39,
    DAV_IDI_IGLVOL1_ = 40,
    DAV_IDI_IGLVOL2_ = 41,
    DAV_IDI_IGLVOL3_ = 42,
    DAV_IDI_IGLVOL4_ = 43,
    DAV_IDI_ILAICONA = 44,
    DAV_IDI_ILAICONB = 45,
    DAV_IDI_IMEMCMAP = 46,
    DAV_IDI_IMOICONA = 47,
    DAV_IDI_IMOICONB = 48,
    DAV_IDI_IVEICONA = 49,
    DAV_IDI_IVEICONB = 50,
    DAV_IDI_MAP = 51,
    WAR_IDO_SAMTRAJ = 52,
    WAR_IDO_SHEEPREPELBOX = 53,
    WAR_IDO_WATERBOX = 54,
    DAV_IDI_IGLCRAI2 = 55,
    DAV_IDI_IENVMAP = 56,
    DAV_IDI_IGLTIDE_ = 57,
    DAV_IDI_IFTICONA = 58,
    DAV_IDI_IFTICONB = 59,
    DAV_IDI_IFLICONA = 60,
    DAV_IDI_IFLICONB = 61,
    DAV_IDI_IGLBULL_ = 62,
    DAV_IDI_IFLFUMEE = 63,
    WAR_IDO_ABCOYO01 = 64,
    DAV_IDI_IGAICONA = 65,
    DAV_IDI_IGAICONB = 66,
    DAV_IDI_IC4ICONA = 67,
    DAV_IDI_IC4ICONB = 68,
    WAR_IDO_BOXSAMGREEN = 69,
    WAR_IDO_BOXSAMORANGE = 70,
    DAV_IDI_ICTJAUG1 = 71,
    DAV_IDI_ICHICONB = 72,
    WAR_IDO_ATEROB01 = 73,
    DAV_IDI_ICHICONA = 74,
    DAV_IDI_IMNICONA = 75,
    DAV_IDI_IMNICONB = 76,
    DAV_IDI_IMNINT1_ = 77,
    DAV_IDI_IMNINT2_ = 78,
    DAV_IDI_IEAICONA = 79,
    DAV_IDI_IEAICONB = 80,
    DAV_IDI_IMNCERC_ = 81,
    WAR_IDO_AMOUTO03 = 82,
    DAV_IDI_IMNPADS_ = 83,
    DAV_IDI_IMNPADC_ = 84,
    DAV_IDI_IMNPADT_ = 85,
    DAV_IDI_IMNPADX_ = 86,
    DAV_IDI_IGLEAVE1 = 87,
    WAR_IDO_AROCHE5B = 88,
    DAV_IDI_IMNINT3_ = 89,
    WAR_IDO_APLOUF01 = 90,
    DAV_IDI_IMNCHEK_ = 91,
    WAR_IDO_ICEBOX = 92,
    DAV_IDI_IAPICONA = 93,
    DAV_IDI_IAPICONB = 94,
    DAV_IDI_IFLOCO01 = 95,
    DAV_IDI_IAIICONA = 96,
    DAV_IDI_IAIICONB = 97,
    WAR_IDO_AGLACON1 = 98,
    WAR_IDO_AMCOYO01 = 99,
    WAR_IDO_AGLACON2 = 100,
    DAV_IDI_IDUICONB = 101,
    DAV_IDI_IPRICONB = 102,
    DAV_IDI_IPRICONA = 103,
    WAR_IDO_SLIDEBOX = 104,
    DAV_IDI_ICLICONA = 105,
    DAV_IDI_ICLICONB = 106,
    DAV_IDI_IDTICONA = 107,
    DAV_IDI_IDTICONB = 108,
    DAV_IDI_ICPICONA = 109,
    DAV_IDI_ICPICONB = 110,
    DAV_IDI_ICPICONC = 111,
    DAV_IDI_IDTICONC = 112,
    DAV_IDI_IDUICONA = 113,
    DAV_IDI_IPMICONA = 114,
    DAV_IDI_IPMICONB = 115,
    WAR_IDO_AFEUIL01 = 116,
    DAV_IDI_IMGICONA = 117,
    DAV_IDI_IMGICONB = 118,
    WAR_IDO_ALAITU02 = 119,
    DAV_IDI_IFFICONA = 120,
    DAV_IDI_IFFICONB = 121,
    DAV_IDI_IPASNE01 = 122,
    WAR_IDO_PRINTSBOX = 123,
    WAR_IDO_ACOLAP01 = 124,
    WAR_IDO_ABTIME02 = 125,
    DAV_IDI_ICFICONA = 126,
    DAV_IDI_ICFICONB = 127,
    WAR_IDO_ATELMER2 = 128,
    WAR_IDO_ABOULE01 = 129,
    DAV_IDI_ITICPTR_ = 130,
    DAV_IDI_IFUJAUG_ = 131,
    DAV_IDI_IFUJIC1_ = 132,
    DAV_IDI_IFUJIC2_ = 133,
    DAV_IDI_ITITRE01 = 134,
    DAV_IDI_ITITRE02 = 135,
    WAR_IDO_ACANON01 = 136,
    WAR_IDO_ACENSURE = 137,
    DAV_IDI_ITRICONA = 138,
    DAV_IDI_ITRICONB = 139,
    DAV_IDI_ITITRE03 = 140,
    DAV_IDI_ITITRE04 = 141,
    DAV_IDI_IGLCOYO1 = 142,
    DAV_IDI_ISTART01 = 143,
    DAV_IDI_ISTART02 = 144,
    DAV_IDI_ISTART03 = 145,
    DAV_IDI_ISTART04 = 146,
    WAR_IDO_AGRILL01 = 147,
    WAR_IDO_AGRILL03 = 148,
    WAR_IDO_AGOOMB01 = 149,
    WAR_IDO_AGRILL02 = 150,
    WAR_IDO_ATAPIS01 = 151,
    DAV_IDI_ITPDESS_ = 152,
    WAR_IDO_GHOSTTRAJ = 153,
    WAR_IDO_ACOGHO01 = 154,
    DAV_IDI_IASPILEA = 155,
    DAV_IDI_IASPILEB = 156,
    DAV_IDI_IASICONA = 157,
    DAV_IDI_IASICONB = 158,
    WAR_IDO_APIRANA1 = 159,
    DAV_IDI_ICSMASQ_ = 160,
    WAR_IDO_AGRILL04 = 161,
    DAV_IDI_ICPICOND = 162,
    DAV_IDI_IORICONA = 163,
    DAV_IDI_IORICONB = 164,
    WAR_IDO_AFANTO03 = 165,
    WAR_IDO_ACAISS02 = 166,
    WAR_IDO_ALOCOM1A = 167,
    WAR_IDO_ALOCOM1B = 168,
    WAR_IDO_MARTIANEXCEPTINGBOX = 169,
    WAR_IDO_ACAISS03 = 170,
    DAV_IDI_IGHICONA = 171,
    DAV_IDI_IGHICONB = 172,
    WAR_IDO_GRAVITYBOX = 173,
    DAV_IDI_D0XCADR_ = 174,
    WAR_IDO_BEACHCSBOX1 = 175,
    WAR_IDO_BEACHCSBOX2 = 176,
    WAR_IDO_BEACHSAMIPOS2 = 177,
    WAR_IDO_AWAGON1A = 178,
    WAR_IDO_AWAGON1B = 179,
    WAR_IDO_AWAGON2A = 180,
    WAR_IDO_AWAGON2B = 181,
    WAR_IDO_AREDUC01 = 182,
    WAR_IDO_AREDUC03 = 183,
    WAR_IDO_APAPIL02 = 184,
    WAR_IDO_APAPIL03 = 185,
    WAR_IDO_ALAVE02 = 186,
    WAR_IDO_ALAVE03 = 187,
    WAR_IDO_ASABLE01 = 188,
    WAR_IDO_AFANTB02 = 189,
    WAR_IDO_AREDUC02 = 190,
    WAR_IDO_AFANTB01 = 191,
    WAR_IDO_AFANTO04 = 192,
    DAV_IDI_IGHICONC = 193,
    DAV_IDI_IDTJAUG_ = 194,
    WAR_IDO_BEACHORANGEZONE2 = 195,
    WAR_IDO_BEACHGREENZONE2 = 196,
    WAR_IDO_SAMSPECIALBOX = 197,
    WAR_IDO_AGRILL05 = 198,
    WAR_IDO_ASABLE02 = 199,
    WAR_IDO_BEACHPSBOX1 = 200,
    WAR_IDO_BEACHPSBOX2 = 201,
    WAR_IDO_SHADOWBOX = 202,
    WAR_IDO_FALLINGGATE = 203,
    WAR_IDO_FALLINGGATE2 = 204,
    WAR_IDO_WHEEL = 205,
    WAR_IDO_WOODENLIFT = 206,
    DAV_IDI_IFAOREO_ = 207,
    DAV_IDI_IFBSMOK_ = 208,
    DAV_IDI_IFTNOTE_ = 209,
    DAV_IDI_IFTTSAM_ = 210,
    WAR_IDO_ACBOUE01 = 211,
    WAR_IDO_AGLACON3 = 212,
    WAR_IDO_ACROCO02 = 213,
    DAV_IDI_IMDANSE1 = 214,
    WAR_IDO_ACHRON01 = 215,
    WAR_IDO_ATIMER01 = 216,
    WAR_IDO_MINTDAN1 = 217,
    DAV_IDI_IMDANSE2 = 218,
    DAV_IDI_IMDANSE3 = 219,
    DAV_IDI_IMDANSE4 = 220,
    WAR_IDO_ATORCH02 = 221,
    WAR_IDO_AROCHE03 = 222,
    WAR_IDO_AEXPLOS3 = 223,
    WAR_IDO_AMINEM02 = 224,
    DAV_IDI_IGLGOUT1 = 225,
    WAR_IDO_ABOUTO01 = 226,
    WAR_IDO_AECLAI01 = 227,
    WAR_IDO_AMOUTO04 = 228,
    DAV_IDI_IBOCKOI_ = 229,
    DAV_IDI_IBOGAGN_ = 230,
    WAR_IDO_AROBIN01 = 231,
    DAV_IDI_IPASNE02 = 232,
    DAV_IDI_IGLGOUT2 = 233,
    WAR_IDO_CLIMATEBOX = 234,
    DAV_IDI_IMEMCICA = 235,
    DAV_IDI_IMEMCICB = 236,
    DAV_IDI_IGLALFA_ = 237,
    DAV_IDI_IGLSMO2_ = 238,
    DAV_IDI_IGLLIGF_ = 239,
    DAV_IDI_IGLLIGB_ = 240,
} GameResId;

// class "Wolf" (id 0), PROPSIZE 12
typedef struct WolfProps {
    u32   CLIMBBOXES;
    u32   FLYBOX;
    u32   RESTRICTIONBOX;
} WolfProps;

// class "Sam" (id 1), PROPSIZE 88
typedef struct SamProps {
    u32   _2NDCAMERABOX;
    u32   AUTHORIZEDZONE;
    u32   CATCHABLESHEEPBOX;
    u32   CIN01;
    u32   CIN01BOX;
    u32   CIN01FLAGS;
    u32   CIN01SHEEPBOX;
    u32   CIN01TEXT;
    u32   DEATHDURATIONMS;
    u32   DISTCATCH_PATROL;
    u32   DNTTRYTOCATCHSHPBOX;
    u32   GREENZONEBOX;
    u32   HEADSPEED;
    u32   HIDINGBOXES;
    u32   HYPNOTIZEDZONE;
    u32   MAXANGLE;
    u32   MODE;
    u32   ORANGEZONEBOX;
    u32   PUTSHEEPBOX;
    u32   TRAJECTORY;
    u32   WATCHROBOTTIMEMS;
    u32   WOLFCANBEHITZONE;
} SamProps;

// class "Dynamite" (id 2), PROPSIZE 4
typedef struct DynamiteProps {
    u32   COUNTDOWN;
} DynamiteProps;

// class "Salad" (id 3), PROPSIZE 0
// (no properties)

// class "Mailbox" (id 4), PROPSIZE 56
typedef struct MailboxProps {
    u32   BOX1;
    u32   BOX2;
    u32   BOX3;
    u32   BOX4;
    u32   FLOATINGBOXES;
    u32   IDCAMERA;
    u32   LVL03ROCK;
    u32   MAPPOSX;
    u32   MAPPOSY;
    u32   O1;
    u32   O2;
    u32   O3;
    u32   O4;
    u32   TIMERCAMERA;
} MailboxProps;

// class "balance" (id 5), PROPSIZE 0
// (no properties)

// class "bridge" (id 6), PROPSIZE 12
typedef struct bridgeProps {
    u32   FALLINGZONE;
    u32   IDOBJECTSFALLZONE;
    u32   OTHERPART;
} bridgeProps;

// class "Fan" (id 7), PROPSIZE 0
// (no properties)

// class "robin" (id 8), PROPSIZE 0
// (no properties)

// class "Porky" (id 9), PROPSIZE 0
// (no properties)

// class "Rocket" (id 10), PROPSIZE 16
typedef struct RocketProps {
    u32   FUEL;
    u32   FUELCOLOR;
    u32   HITS;
    u32   HITSCOLOR;
} RocketProps;

// class "Sheep" (id 11), PROPSIZE 4
typedef struct SheepProps {
    u32   SLEEPY;
} SheepProps;

// class "MiscStatic" (id 12), PROPSIZE 0
// (no properties)

// class "Rocks" (id 13), PROPSIZE 0
// (no properties)

// class "box" (id 14), PROPSIZE 4
typedef struct boxProps {
    u32   CONTAINS;
} boxProps;

// class "seesaw" (id 15), PROPSIZE 52
typedef struct seesawProps {
    u32   CAMERA1;
    u32   CAMERA1INTERPB;
    u32   CAMERA1INTERPE;
    u32   CAMERA2;
    u32   CAMERA2INTERPB;
    u32   CAMERA2INTERPE;
    u32   EJECTIONBOX1;
    u32   EJECTIONBOX2;
    u32   FLAGS;
    u32   MAXANGLE;
    u32   MAXANGLE2;
    u32   TRAJ1;
    u32   TRAJ2;
} seesawProps;

// class "Rock" (id 16), PROPSIZE 48
typedef struct RockProps {
    u32   ALLOWPUSHBOXES;
    u32   ALLOWPUSHONXAXIS;
    u32   ALLOWPUSHONYAXIS;
    u32   FALLSATINIT;
    u32   LVL03BOX;
    u32   LVL03BOX01;
    u32   LVL03BOX02;
    u32   LVL03IDCAMERABOX;
    u32   LVL03SEESAW;
    u32   LVL03WITHBOX;
    u32   LVL03WITHOUTBOX;
    u32   RESET;
} RockProps;

// class "Tree_02" (id 17), PROPSIZE 0
// (no properties)

// class "CameraManager" (id 18), PROPSIZE 240
typedef struct CameraManagerProps {
    u32   CAMERA01;
    u32   CAMERA01BOX;
    u32   CAMERA01INTB;
    u32   CAMERA01INTE;
    u32   CAMERA02;
    u32   CAMERA02BOX;
    u32   CAMERA02INTB;
    u32   CAMERA02INTE;
    u32   CAMERA03;
    u32   CAMERA03BOX;
    u32   CAMERA03INTB;
    u32   CAMERA03INTE;
    u32   CAMERA04;
    u32   CAMERA04BOX;
    u32   CAMERA04INTB;
    u32   CAMERA04INTE;
    u32   CAMERA05;
    u32   CAMERA05BOX;
    u32   CAMERA05INTB;
    u32   CAMERA05INTE;
    u32   CAMERA06;
    u32   CAMERA06BOX;
    u32   CAMERA06INTB;
    u32   CAMERA06INTE;
    u32   CAMERA07;
    u32   CAMERA07BOX;
    u32   CAMERA07INTB;
    u32   CAMERA07INTE;
    u32   CAMERA08;
    u32   CAMERA08BOX;
    u32   CAMERA08INTB;
    u32   CAMERA08INTE;
    u32   CAMERA09;
    u32   CAMERA09BOX;
    u32   CAMERA09INTB;
    u32   CAMERA09INTE;
    u32   CAMERA10;
    u32   CAMERA10BOX;
    u32   CAMERA10INTB;
    u32   CAMERA10INTE;
    u32   CAMERA11;
    u32   CAMERA11BOX;
    u32   CAMERA11INTB;
    u32   CAMERA11INTE;
    u32   CAMERA12;
    u32   CAMERA12BOX;
    u32   CAMERA12INTB;
    u32   CAMERA12INTE;
    u32   CAMERA13;
    u32   CAMERA13BOX;
    u32   CAMERA13INTB;
    u32   CAMERA13INTE;
    u32   CAMERA14;
    u32   CAMERA14BOX;
    u32   CAMERA14INTB;
    u32   CAMERA14INTE;
    u32   CAMERA15;
    u32   CAMERA15BOX;
    u32   CAMERA15INTB;
    u32   CAMERA15INTE;
} CameraManagerProps;

// class "CameraRestriction" (id 19), PROPSIZE 88
typedef struct CameraRestrictionProps {
    u32   AIMCORRECTIONX;
    u32   AIMCORRECTIONY;
    u32   AIMCORRECTIONZ;
    u32   ALPHAMAX;
    u32   ALPHAMIN;
    u32   ALPHASPEED;
    u32   ALTITUDEMIN;
    u32   BETAMAX;
    u32   BETAMIN;
    u32   BETASPEED;
    u32   DISTMAX;
    u32   DISTMIN;
    u32   FOCAL;
    u32   FORBIDPADCONTROL;
    u32   GAMMAMAX;
    u32   GAMMAMIN;
    u32   GAMMASPEED;
    u32   INFLUENCEBOXES;
    u32   MIRRORBETAINTERVAL;
    u32   MISCFLAGS;
    u32   SNAPBETA;
    u32   TRAJECTORY;
} CameraRestrictionProps;

// class "FallingRock" (id 20), PROPSIZE 12
typedef struct FallingRockProps {
    u32   ACTIVATIONBOX;
    u32   CAMERA;
    u32   TIMERCAMERA;
} FallingRockProps;

// class "SignPost" (id 21), PROPSIZE 16
typedef struct SignPostProps {
    u32   DISTANCE;
    u32   ID_ANVIL;
    u32   ID_CAMERA;
    u32   TEXTNUM;
} SignPostProps;

// class "DaffyTrainingLevel" (id 22), PROPSIZE 456
typedef struct DaffyTrainingLevelProps {
    u32   BOX_1;
    u32   BOX_2;
    u32   BOX_3;
    u32   BOX00;
    u32   BOX01;
    u32   BOX02;
    u32   BOX03;
    u32   BOX04;
    u32   BOX05;
    u32   BOX06;
    u32   BOX07;
    u32   BOX08;
    u32   BOX09;
    u32   BOX10;
    u32   BOX11;
    u32   BOX12;
    u32   BOX13;
    u32   BOX14;
    u32   BOX15;
    u32   BOX16;
    u32   CAMERAS;
    u32   CINEMATICS;
    u32   DAFFYHELPCAMS;
    u32   FROMPREVIOUS;
    u32   HELPBOX;
    u32   HELPTEXT;
    u32   HELPTRAJECTORIES;
    u32   HELPVOICE;
    u32   JOKEENABLED;
    u32   JOKEVOICE1;
    u32   JOKEVOICE2;
    u32   RACEBEGIN;
    u32   RACEEND;
    u32   RACETIME;
    u32   TEXT_1;
    u32   TEXT_2;
    u32   TEXT_3;
    u32   TEXT00;
    u32   TEXT01;
    u32   TEXT02;
    u32   TEXT03;
    u32   TEXT04;
    u32   TEXT05;
    u32   TEXT06;
    u32   TEXT07;
    u32   TEXT08;
    u32   TEXT09;
    u32   TEXT10;
    u32   TEXT11;
    u32   TEXT12;
    u32   TEXT13;
    u32   TEXT14;
    u32   TEXT15;
    u32   TEXT16;
    u32   TRAJECTORY_1;
    u32   TRAJECTORY_2;
    u32   TRAJECTORY_3;
    u32   TRAJECTORY00;
    u32   TRAJECTORY01;
    u32   TRAJECTORY02;
    u32   TRAJECTORY03;
    u32   TRAJECTORY04;
    u32   TRAJECTORY05;
    u32   TRAJECTORY06;
    u32   TRAJECTORY07;
    u32   TRAJECTORY08;
    u32   TRAJECTORY09;
    u32   TRAJECTORY10;
    u32   TRAJECTORY11;
    u32   TRAJECTORY12;
    u32   TRAJECTORY13;
    u32   TRAJECTORY14;
    u32   TRAJECTORY15;
    u32   TRAJECTORY16;
    u32   TYPE_1;
    u32   TYPE_2;
    u32   TYPE_3;
    u32   TYPE00;
    u32   TYPE01;
    u32   TYPE02;
    u32   TYPE03;
    u32   TYPE04;
    u32   TYPE05;
    u32   TYPE06;
    u32   TYPE07;
    u32   TYPE08;
    u32   TYPE09;
    u32   TYPE10;
    u32   TYPE11;
    u32   TYPE12;
    u32   TYPE13;
    u32   TYPE14;
    u32   TYPE15;
    u32   TYPE16;
    u32   VOICE_1;
    u32   VOICE_2;
    u32   VOICE_3;
    u32   VOICE00;
    u32   VOICE01;
    u32   VOICE02;
    u32   VOICE03;
    u32   VOICE04;
    u32   VOICE05;
    u32   VOICE06;
    u32   VOICE07;
    u32   VOICE08;
    u32   VOICE09;
    u32   VOICE10;
    u32   VOICE11;
    u32   VOICE12;
    u32   VOICE13;
    u32   VOICE14;
    u32   VOICE15;
    u32   VOICE16;
} DaffyTrainingLevelProps;

// class "Goal" (id 23), PROPSIZE 24
typedef struct GoalProps {
    u32   CINBOX;
    u32   CINEMATIC;
    u32   CINSHEEPBOX;
    u32   FLAGS;
    u32   FLAGSFORCIN;
    u32   TEXTFORCIN;
} GoalProps;

// class "SensibleButton" (id 24), PROPSIZE 12
typedef struct SensibleButtonProps {
    u32   TARGET;
    u32   TARGET2;
    u32   TYPE;
} SensibleButtonProps;

// class "CheckpointManager" (id 25), PROPSIZE 132
typedef struct CheckpointManagerProps {
    u32   CHECK01_BOXES;
    u32   CHECK01_ORIENTATION;
    u32   CHECK01_SHEEP;
    u32   CHECK01_WOLF;
    u32   CHECK02_BOXES;
    u32   CHECK02_ORIENTATION;
    u32   CHECK02_SHEEP;
    u32   CHECK02_WOLF;
    u32   CHECK03_BOXES;
    u32   CHECK03_ORIENTATION;
    u32   CHECK03_SHEEP;
    u32   CHECK03_WOLF;
    u32   CHECK04_BOXES;
    u32   CHECK04_ORIENTATION;
    u32   CHECK04_SHEEP;
    u32   CHECK04_WOLF;
    u32   CHECK05_BOXES;
    u32   CHECK05_ORIENTATION;
    u32   CHECK05_SHEEP;
    u32   CHECK05_WOLF;
    u32   CHECK06_BOXES;
    u32   CHECK06_ORIENTATION;
    u32   CHECK06_SHEEP;
    u32   CHECK06_WOLF;
    u32   CHECK07_BOXES;
    u32   CHECK07_ORIENTATION;
    u32   CHECK07_SHEEP;
    u32   CHECK07_WOLF;
    u32   CHECK08_BOXES;
    u32   CHECK08_ORIENTATION;
    u32   CHECK08_SHEEP;
    u32   CHECK08_WOLF;
    u32   SHEEPANDWOLFONLY;
} CheckpointManagerProps;

// class "FallingGate" (id 26), PROPSIZE 20
typedef struct FallingGateProps {
    u32   CLOSED;
    u32   CUTCAMERA;
    u32   IDCAMERA;
    u32   ONLYONEACTIVATION;
    u32   TYPE;
} FallingGateProps;

// class "Cactus" (id 27), PROPSIZE 0
// (no properties)

// class "Telescope" (id 28), PROPSIZE 40
typedef struct TelescopeProps {
    u32   CAMDISTNORMAL;
    u32   CAMDISTREVERSE;
    u32   INITHORANGLE;
    u32   INITVERTANGLE;
    u32   MAXHORANGLE;
    u32   MAXVERTANGLE;
    u32   NORMALBOXID;
    u32   NORMALFOCAL;
    u32   REVERSEBOXID;
    u32   REVERSEFOCAL;
} TelescopeProps;

// class "Anvil" (id 29), PROPSIZE 0
// (no properties)

// class "PorkyLevel01" (id 30), PROPSIZE 52
typedef struct PorkyLevel01Props {
    u32   ACTIVATIONBOX;
    u32   ANSWERSTEXTS;
    u32   ASKTEXT;
    u32   CHOICESTEXTS;
    u32   CINBOX;
    u32   CINWITHOUTSHEEP;
    u32   CINWITHOUTSHEEPTEXT;
    u32   CINWITHSHEEP;
    u32   CINWITHSHEEPTEXT;
    u32   DISPLAYSALADMILLIS;
    u32   SHEEPBOX;
    u32   WITHOUTSHEEPTEXT2;
    u32   WITHSHEEPTEXT2;
} PorkyLevel01Props;

// class "bipbip" (id 31), PROPSIZE 20
typedef struct bipbipProps {
    u32   BOX_DECL;
    u32   SPEED_ONE;
    u32   SPEED_TWO;
    u32   TRAJ_WALL;
    u32   TRAJECTORY;
} bipbipProps;

// class "CinematicsManager" (id 32), PROPSIZE 360
typedef struct CinematicsManagerProps {
    u32   CIN01;
    u32   CIN01ACTBOX;
    u32   CIN01BOX;
    u32   CIN01FLAGS;
    u32   CIN01SHEEPBOX;
    u32   CIN01TEXT;
    u32   CIN02;
    u32   CIN02ACTBOX;
    u32   CIN02BOX;
    u32   CIN02FLAGS;
    u32   CIN02SHEEPBOX;
    u32   CIN02TEXT;
    u32   CIN03;
    u32   CIN03ACTBOX;
    u32   CIN03BOX;
    u32   CIN03FLAGS;
    u32   CIN03SHEEPBOX;
    u32   CIN03TEXT;
    u32   CIN04;
    u32   CIN04ACTBOX;
    u32   CIN04BOX;
    u32   CIN04FLAGS;
    u32   CIN04SHEEPBOX;
    u32   CIN04TEXT;
    u32   CIN05;
    u32   CIN05ACTBOX;
    u32   CIN05BOX;
    u32   CIN05FLAGS;
    u32   CIN05SHEEPBOX;
    u32   CIN05TEXT;
    u32   CIN06;
    u32   CIN06ACTBOX;
    u32   CIN06BOX;
    u32   CIN06FLAGS;
    u32   CIN06SHEEPBOX;
    u32   CIN06TEXT;
    u32   CIN07;
    u32   CIN07ACTBOX;
    u32   CIN07BOX;
    u32   CIN07FLAGS;
    u32   CIN07SHEEPBOX;
    u32   CIN07TEXT;
    u32   CIN08;
    u32   CIN08ACTBOX;
    u32   CIN08BOX;
    u32   CIN08FLAGS;
    u32   CIN08SHEEPBOX;
    u32   CIN08TEXT;
    u32   CIN09;
    u32   CIN09ACTBOX;
    u32   CIN09BOX;
    u32   CIN09FLAGS;
    u32   CIN09SHEEPBOX;
    u32   CIN09TEXT;
    u32   CIN10;
    u32   CIN10ACTBOX;
    u32   CIN10BOX;
    u32   CIN10FLAGS;
    u32   CIN10SHEEPBOX;
    u32   CIN10TEXT;
    u32   CIN11;
    u32   CIN11ACTBOX;
    u32   CIN11BOX;
    u32   CIN11FLAGS;
    u32   CIN11SHEEPBOX;
    u32   CIN11TEXT;
    u32   CIN12;
    u32   CIN12ACTBOX;
    u32   CIN12BOX;
    u32   CIN12FLAGS;
    u32   CIN12SHEEPBOX;
    u32   CIN12TEXT;
    u32   CIN13;
    u32   CIN13ACTBOX;
    u32   CIN13BOX;
    u32   CIN13FLAGS;
    u32   CIN13SHEEPBOX;
    u32   CIN13TEXT;
    u32   CIN14;
    u32   CIN14ACTBOX;
    u32   CIN14BOX;
    u32   CIN14FLAGS;
    u32   CIN14SHEEPBOX;
    u32   CIN14TEXT;
    u32   CIN15;
    u32   CIN15ACTBOX;
    u32   CIN15BOX;
    u32   CIN15FLAGS;
    u32   CIN15SHEEPBOX;
    u32   CIN15TEXT;
} CinematicsManagerProps;

// class "DaffyLevel01" (id 33), PROPSIZE 52
typedef struct DaffyLevel01Props {
    u32   ANSWERTEXTS;
    u32   CHOICETEXTS;
    u32   LEVEL;
    u32   NBCHOICES;
    u32   QUESTIONTEXT;
    u32   TEXTBURNT;
    u32   TEXTCAMERA;
    u32   VOICE1;
    u32   VOICE2;
    u32   VOICE3;
    u32   VOICE4;
    u32   VOICE5;
    u32   VOICEBURNT;
} DaffyLevel01Props;

// class "TVSceneManager" (id 34), PROPSIZE 480
typedef struct TVSceneManagerProps {
    u32   CINE00;
    u32   CINE00FLAGS;
    u32   CINE00TEXT;
    u32   CINE01;
    u32   CINE01FLAGS;
    u32   CINE01TEXT;
    u32   CINE02;
    u32   CINE02FLAGS;
    u32   CINE02TEXT;
    u32   CINE03;
    u32   CINE03FLAGS;
    u32   CINE03TEXT;
    u32   CINE04;
    u32   CINE04FLAGS;
    u32   CINE04TEXT;
    u32   CINE05;
    u32   CINE05FLAGS;
    u32   CINE05TEXT;
    u32   CINE06;
    u32   CINE06FLAGS;
    u32   CINE06TEXT;
    u32   CINE07;
    u32   CINE07FLAGS;
    u32   CINE07TEXT;
    u32   CINE08;
    u32   CINE08FLAGS;
    u32   CINE08TEXT;
    u32   CINE09;
    u32   CINE09FLAGS;
    u32   CINE09TEXT;
    u32   CINE10;
    u32   CINE10FLAGS;
    u32   CINE10TEXT;
    u32   CINE11;
    u32   CINE11FLAGS;
    u32   CINE11TEXT;
    u32   CINE12;
    u32   CINE12FLAGS;
    u32   CINE12TEXT;
    u32   CINE13;
    u32   CINE13FLAGS;
    u32   CINE13TEXT;
    u32   CINE14;
    u32   CINE14FLAGS;
    u32   CINE14TEXT;
    u32   CINE15;
    u32   CINE15FLAGS;
    u32   CINE15TEXT;
    u32   CINE16;
    u32   CINE16FLAGS;
    u32   CINE16TEXT;
    u32   CINE17;
    u32   CINE17FLAGS;
    u32   CINE17TEXT;
    u32   CINE18;
    u32   CINE18FLAGS;
    u32   CINE18TEXT;
    u32   CINE19;
    u32   CINE19FLAGS;
    u32   CINE19TEXT;
    u32   CINE20;
    u32   CINE20FLAGS;
    u32   CINE20TEXT;
    u32   CINE21;
    u32   CINE21FLAGS;
    u32   CINE21TEXT;
    u32   CINEINTRO;
    u32   CINEINTROFLAGS;
    u32   CINEINTROTEXT;
    u32   EXEDEMO1;
    u32 _pad0[1];
    u32   EXEDEMO2;
    u32 _pad1[1];
    u32   EXELEV00;
    u32 _pad2[1];
    u32   EXELEV01;
    u32 _pad3[1];
    u32   EXELEV02;
    u32 _pad4[1];
    u32   EXELEV03;
    u32 _pad5[1];
    u32   EXELEV04;
    u32 _pad6[1];
    u32   EXELEV05;
    u32 _pad7[1];
    u32   EXELEV06;
    u32 _pad8[1];
    u32   EXELEV07;
    u32 _pad9[1];
    u32   EXELEV08;
    u32 _pad10[1];
    u32   EXELEV09;
    u32 _pad11[1];
    u32   EXELEV10;
    u32 _pad12[1];
    u32   EXELEV11;
    u32 _pad13[1];
    u32   EXELEV12;
    u32 _pad14[1];
    u32   EXELEV13;
    u32 _pad15[1];
    u32   EXELEV14;
    u32 _pad16[1];
    u32   EXELEV15;
    u32 _pad17[1];
    u32   EXELEV16;
    u32 _pad18[1];
    u32   EXELEV17;
    u32 _pad19[1];
    u32   EXELEV18;
    u32 _pad20[1];
    u32   EXELEV19;
    u32 _pad21[1];
    u32   EXELEV20;
    u32 _pad22[1];
    u32   EXELEV21;
    u32 _pad23[1];
    u32   EXELEV22;
    u32 _pad24[1];
    u32   GLOBALFLAGS;
} TVSceneManagerProps;

// class "HiddenRocks" (id 35), PROPSIZE 4
typedef struct HiddenRocksProps {
    u32   IDHIDDINGZONE;
} HiddenRocksProps;

// class "Wooden Lift" (id 36), PROPSIZE 20
typedef struct WoodenLiftProps {
    u32   CAMERA;
    u32   CAMERACHANGEZVALUE;
    u32   IDCAMERABOX;
    u32   MAXZVALUE;
    u32   STEP;
} WoodenLiftProps;

// class "GoldenRock" (id 37), PROPSIZE 20
typedef struct GoldenRockProps {
    u32   BOX;
    u32   EARTH;
    u32   FLAGEARTH;
    u32   FLAGSPACE;
    u32   SPACE;
} GoldenRockProps;

// class "TimeKeeper" (id 38), PROPSIZE 0
// (no properties)

// class "Perfume" (id 39), PROPSIZE 0
// (no properties)

// class "WoodenPlatForm" (id 40), PROPSIZE 24
typedef struct WoodenPlatFormProps {
    u32   ACTIVATIONZONE;
    u32   CAMERA;
    u32   DOOR;
    u32   IDMECHANISM;
    u32   MINZVALUE;
    u32   STEP;
} WoodenPlatFormProps;

// class "Flute" (id 41), PROPSIZE 0
// (no properties)

// class "elastic" (id 42), PROPSIZE 20
typedef struct elasticProps {
    u32   DISTANCE_PROJECTION;
    u32   HAUTEUR_PROJECTION;
    u32   IDCAMERA;
    u32   LONGUEUR_MAX;
    u32   VITESSE_PROJECTION;
} elasticProps;

// class "bull" (id 43), PROPSIZE 36
typedef struct bullProps {
    u32   ACTIVATIONBOX;
    u32   ASLEEP;
    u32   MOVEMENTBOX;
    u32   NORETURNBOX;
    u32   REDOBJECTS;
    u32   SHEEPBOX;
    u32   SHEEPCAM;
    u32   SHEEPTRAJ;
    u32   WAKEUPSPEED;
} bullProps;

// class "SecretDoor" (id 44), PROPSIZE 0
// (no properties)

// class "ElasticTree" (id 45), PROPSIZE 8
typedef struct ElasticTreeProps {
    u32   IDCAMREST;
    u32   IDPOINTUP;
} ElasticTreeProps;

// class "ElasticHook" (id 46), PROPSIZE 0
// (no properties)

// class "DaffyLevel02" (id 47), PROPSIZE 40
typedef struct DaffyLevel02Props {
    u32   ACTIVATIONBOX;
    u32   CINE;
    u32   CINE2;
    u32   CINEBOX;
    u32   CINEFLAGS;
    u32   CINESHEEPBOX;
    u32   CINETEXT;
    u32   GOALBOX;
    u32   STARTBOX;
    u32   TEXTBURNT;
} DaffyLevel02Props;

// class "Bush" (id 48), PROPSIZE 0
// (no properties)

// class "RedThing" (id 49), PROPSIZE 0
// (no properties)

// class "Shark" (id 50), PROPSIZE 20
typedef struct SharkProps {
    u32   BOOLWATCHOVERCAVE;
    u32   BOX;
    u32   DETECTHEIGHT;
    u32   DETECTRANGE;
    u32   TRAJ;
} SharkProps;

// class "RedScarf" (id 51), PROPSIZE 0
// (no properties)

// class "TriggedStone" (id 52), PROPSIZE 12
typedef struct TriggedStoneProps {
    u32   CRASHINGZONE;
    u32   IDCAMERA;
    u32   TIMER;
} TriggedStoneProps;

// class "Raft" (id 53), PROPSIZE 4
typedef struct RaftProps {
    u32   RESET;
} RaftProps;

// class "HeapOfLeaf" (id 54), PROPSIZE 12
typedef struct HeapOfLeafProps {
    u32   GLOBALZONE;
    u32   IDGUARDIAN;
    u32   IDPOLY;
} HeapOfLeafProps;

// class "DaffyElf" (id 55), PROPSIZE 32
typedef struct DaffyElfProps {
    u32   ANSWERNUMBER;
    u32   ANSWERTEXTINDEX;
    u32   CINE;
    u32   CINEBOX;
    u32   CINEFLAGS;
    u32   CINESHEEPBOX;
    u32   CINETEXT;
    u32   IDFLUTE;
} DaffyElfProps;

// class "Catapult" (id 56), PROPSIZE 40
typedef struct CatapultProps {
    u32   ACTIVATIONBOX;
    u32   ARMLENGTH1;
    u32   ARMLENGTH2;
    u32   ARMLENGTH3;
    u32   H0;
    u32   MAXDIST;
    u32   MOBILECAMERA;
    u32   MOBILECAMERADIST;
    u32   OFFSET0;
    u32   TRAJECTORY;
} CatapultProps;

// class "DaffyMilitary" (id 57), PROPSIZE 24
typedef struct DaffyMilitaryProps {
    u32   CINE;
    u32   CINETXT;
    u32   FLAGCINE;
    u32   PUNISHTEXT;
    u32   TALKABOUTNBCHOICE;
    u32   TALKABOUTTXT;
} DaffyMilitaryProps;

// class "HairDryer" (id 58), PROPSIZE 0
// (no properties)

// class "DoorMechanism" (id 59), PROPSIZE 0
// (no properties)

// class "IceCube" (id 60), PROPSIZE 4
typedef struct IceCubeProps {
    u32   OBJ;
} IceCubeProps;

// class "MineDetector" (id 61), PROPSIZE 0
// (no properties)

// class "Umbrella" (id 62), PROPSIZE 0
// (no properties)

// class "Stump" (id 63), PROPSIZE 0
// (no properties)

// class "DefusableMine" (id 64), PROPSIZE 16
typedef struct DefusableMineProps {
    u32   CODE;
    u32   CODEINIT;
    u32   CODELENGTH;
    u32   CODESPEED;
} DefusableMineProps;

// class "SmallRock" (id 65), PROPSIZE 12
typedef struct SmallRockProps {
    u32   FALLSATINIT;
    u32   INITATRESET;
    u32   SLIDINGBOX;
} SmallRockProps;

// class "Magnet" (id 66), PROPSIZE 0
// (no properties)

// class "FishingRod" (id 67), PROPSIZE 0
// (no properties)

// class "MagnetRod" (id 68), PROPSIZE 0
// (no properties)

// class "Seaweed" (id 69), PROPSIZE 0
// (no properties)

// class "VisibilityManager" (id 70), PROPSIZE 28
typedef struct VisibilityManagerProps {
    u32   CANTSEE1;
    u32   CANTSEE2;
    u32   CANTSEE3;
    u32   WHENINBOX;
    u32   WHENINBOX2;
    u32   WHENINBOX3;
    u32   WHENINBOX4;
} VisibilityManagerProps;

// class "HitSwitch" (id 71), PROPSIZE 4
typedef struct HitSwitchProps {
    u32   TARGETS;
} HitSwitchProps;

// class "GeyserIn" (id 72), PROPSIZE 36
typedef struct GeyserInProps {
    u32   ALWAYSMANAGE;
    u32   BOXDETECT;
    u32   CAMERA;
    u32   PERIODINSEC;
    u32   SLAVE;
    u32   TARGET;
    u32   TIMEENDOFCORK;
    u32   TIMEINPIPEINSEC;
    u32   TIMETOINHALE;
} GeyserInProps;

// class "GeyserOut" (id 73), PROPSIZE 28
typedef struct GeyserOutProps {
    u32   ALWAYSSPIT;
    u32   CAMERA;
    u32   CAMERAFORSALAD;
    u32   CAMINTERPOLATE;
    u32   SPITBOX;
    u32   TRAJECTORY;
    u32   TRAJECTORYWOLF;
} GeyserOutProps;

// class "FogManager" (id 74), PROPSIZE 28
typedef struct FogManagerProps {
    u32   AUTOCLEAR;
    u32   FOG2_COLOR;
    u32   GRUGRU_COLOR;
    u32   MAX;
    u32   MIN;
    u32   PAUSE_COLOR;
    u32   WATER_COLOR;
} FogManagerProps;

// class "Snowball" (id 75), PROPSIZE 4
typedef struct SnowballProps {
    u32   ZONE;
} SnowballProps;

// class "SnowyGround" (id 76), PROPSIZE 0
// (no properties)

// class "IceGround" (id 77), PROPSIZE 4
typedef struct IceGroundProps {
    u32   IDCAMERA;
} IceGroundProps;

// class "SignPostSimple" (id 78), PROPSIZE 12
typedef struct SignPostSimpleProps {
    u32   DISTANCE;
    u32   HIDDEN;
    u32   INDEXTEXT;
} SignPostSimpleProps;

// class "SlidingIceCube" (id 79), PROPSIZE 12
typedef struct SlidingIceCubeProps {
    u32   ALLOWPUSHBOXES;
    u32   ALLOWPUSHONXAXIS;
    u32   ALLOWPUSHONYAXIS;
} SlidingIceCubeProps;

// class "SheepCostume" (id 80), PROPSIZE 0
// (no properties)

// class "WolfTrap" (id 81), PROPSIZE 0
// (no properties)

// class "Crane" (id 82), PROPSIZE 28
typedef struct CraneProps {
    u32   ACTIVATIONBOX;
    u32   CAMERA;
    u32   CINE;
    u32   CINEBOX;
    u32   CINEFLAG;
    u32   CINESHEEPBOX;
    u32   CINETEXT;
} CraneProps;

// class "FrozenRiver" (id 83), PROPSIZE 12
typedef struct FrozenRiverProps {
    u32   CAMERA;
    u32   IDWATERBOX;
    u32   IDWATERTRAJ;
} FrozenRiverProps;

// class "Dragon" (id 84), PROPSIZE 48
typedef struct DragonProps {
    u32   BOXFIRSTCONTACT;
    u32   BOXFLY1;
    u32   BOXFLY2;
    u32   BOXFLY3;
    u32   BOXFLY4;
    u32   BOXFLY5;
    u32   CINECHRONOBALL;
    u32   CINEFIRSTCONTACT;
    u32   FLAGCHRONOBALL;
    u32   FLAGFIRSTCONTACT;
    u32   ROCKZONE;
    u32   ZONE;
} DragonProps;

// class "TimeMachineChrono" (id 85), PROPSIZE 0
// (no properties)

// class "TimeMachineSphere" (id 86), PROPSIZE 136
typedef struct TimeMachineSphereProps {
    u32   BOXPAST;
    u32   BOXPRESENT;
    u32   OBJECT00PAST;
    u32   OBJECT00PRESENT;
    u32   OBJECT01PAST;
    u32   OBJECT01PRESENT;
    u32   OBJECT02PAST;
    u32   OBJECT02PRESENT;
    u32   OBJECT03PAST;
    u32   OBJECT03PRESENT;
    u32   OBJECT04PAST;
    u32   OBJECT04PRESENT;
    u32   OBJECT05PAST;
    u32   OBJECT05PRESENT;
    u32   OBJECT06PAST;
    u32   OBJECT06PRESENT;
    u32   OBJECT07PAST;
    u32   OBJECT07PRESENT;
    u32   OBJECT08PAST;
    u32   OBJECT08PRESENT;
    u32   OBJECT09PAST;
    u32   OBJECT09PRESENT;
    u32   OBJECT10PAST;
    u32   OBJECT10PRESENT;
    u32   OBJECT11PAST;
    u32   OBJECT11PRESENT;
    u32   OBJECT12PAST;
    u32   OBJECT12PRESENT;
    u32   OBJECT13PAST;
    u32   OBJECT13PRESENT;
    u32   OBJECT14PAST;
    u32   OBJECT14PRESENT;
    u32   OBJECT15PAST;
    u32   OBJECT15PRESENT;
} TimeMachineSphereProps;

// class "Gossamer_Lev08" (id 87), PROPSIZE 56
typedef struct Gossamer_Lev08Props {
    u32   CINEFLAGS;
    u32   CINEFLAGS2;
    u32   CINETEXT;
    u32   CINETEXT2;
    u32   IDBULL;
    u32   IDCINEBOX;
    u32   IDCINEBOX2;
    u32   IDCINEMATIC;
    u32   IDCINEMATIC2;
    u32   IDFLEEINGTRAJ;
    u32   IDFUTURACTIONZONE;
    u32   IDFUTURACTIONZONE2;
    u32   IDFUTURACTIONZONE3;
    u32   IDPASTACTIONZONE;
} Gossamer_Lev08Props;

// class "GroundMine" (id 88), PROPSIZE 16
typedef struct GroundMineProps {
    u32   BOXES;
    u32   RADIUS;
    u32   TIMEINS;
    u32   TRAJECTORIES;
} GroundMineProps;

// class "SwirlSign" (id 89), PROPSIZE 4
typedef struct SwirlSignProps {
    u32   ISRABBIT;
} SwirlSignProps;

// class "Elmer" (id 90), PROPSIZE 40
typedef struct ElmerProps {
    u32   BULLET;
    u32   CAMERA;
    u32   DAFFYTEXT;
    u32   DETECTBOX;
    u32   EXCEPTMOVINGBOX;
    u32   MOVINGBOX;
    u32   RABBITTEXT;
    u32   SWIRLSIGN;
    u32   TRAJECTORY;
    u32   TREESECTION;
} ElmerProps;

// class "TreeSection" (id 91), PROPSIZE 0
// (no properties)

// class "Bullet" (id 92), PROPSIZE 0
// (no properties)

// class "Seed" (id 93), PROPSIZE 8
typedef struct SeedProps {
    u32   BOXES;
    u32   TREE;
} SeedProps;

// class "Tree" (id 94), PROPSIZE 4
typedef struct TreeProps {
    u32   VISIBLEDIRECTLY;
} TreeProps;

// class "CannonBall" (id 95), PROPSIZE 12
typedef struct CannonBallProps {
    u32   LIFETIME;
    u32   PARABOLIC;
    u32   WEIGHT;
} CannonBallProps;

// class "Pipe" (id 96), PROPSIZE 20
typedef struct PipeProps {
    u32   BOXACTI1;
    u32   BOXACTI2;
    u32   BOXLOCKUP;
    u32   FORBID_IN_X;
    u32   FORBID_IN_Z;
} PipeProps;

// class "CanonSimple" (id 97), PROPSIZE 28
typedef struct CanonSimpleProps {
    u32   _ACTIVATIONBOX;
    u32   ANGLEX;
    u32   ANGLEY;
    u32   CANNONBALL;
    u32   CANONDUMMY;
    u32   HORZLIMIT;
    u32   VERTLIMIT;
} CanonSimpleProps;

// class "MCardManager" (id 98), PROPSIZE 4
typedef struct MCardManagerProps {
    u32   ID_CAMERA;
} MCardManagerProps;

// class "RabbitCostume" (id 99), PROPSIZE 0
// (no properties)

// class "RemoteControl" (id 100), PROPSIZE 8
typedef struct RemoteControlProps {
    u32   SWITCHONOFF;
    u32   TARGET;
} RemoteControlProps;

// class "Robot" (id 101), PROPSIZE 8
typedef struct RobotProps {
    u32   EJECTCAM;
    u32   EJECTTRAJ;
} RobotProps;

// class "Bell" (id 102), PROPSIZE 0
// (no properties)

// class "Watch" (id 103), PROPSIZE 0
// (no properties)

// class "DaffyLevel09" (id 104), PROPSIZE 52
typedef struct DaffyLevel09Props {
    u32   ACTIVATIONBOX;
    u32   BOXDEGUIS;
    u32   CAMERATRAJ;
    u32   CINE;
    u32   CINEBOX;
    u32   CINEDEGUIS;
    u32   CINEDEGUISFLAG;
    u32   CINEFLAG;
    u32   CINESHEEPBOX;
    u32   CINETEXT;
    u32   CINETEXTDEGUIS;
    u32   TRAJ;
    u32   TRAJRET;
} DaffyLevel09Props;

// class "CanonDummy" (id 105), PROPSIZE 0
// (no properties)

// class "MapLocation" (id 106), PROPSIZE 180
typedef struct MapLocationProps {
    u32   BOX01;
    u32   BOX01POSX;
    u32   BOX01POSY;
    u32   BOX02;
    u32   BOX02POSX;
    u32   BOX02POSY;
    u32   BOX03;
    u32   BOX03POSX;
    u32   BOX03POSY;
    u32   BOX04;
    u32   BOX04POSX;
    u32   BOX04POSY;
    u32   BOX05;
    u32   BOX05POSX;
    u32   BOX05POSY;
    u32   BOX06;
    u32   BOX06POSX;
    u32   BOX06POSY;
    u32   BOX07;
    u32   BOX07POSX;
    u32   BOX07POSY;
    u32   BOX08;
    u32   BOX08POSX;
    u32   BOX08POSY;
    u32   BOX09;
    u32   BOX09POSX;
    u32   BOX09POSY;
    u32   BOX10;
    u32   BOX10POSX;
    u32   BOX10POSY;
    u32   BOX11;
    u32   BOX11POSX;
    u32   BOX11POSY;
    u32   BOX12;
    u32   BOX12POSX;
    u32   BOX12POSY;
    u32   BOX13;
    u32   BOX13POSX;
    u32   BOX13POSY;
    u32   BOX14;
    u32   BOX14POSX;
    u32   BOX14POSY;
    u32   BOX15;
    u32   BOX15POSX;
    u32   BOX15POSY;
} MapLocationProps;

// class "Bees" (id 107), PROPSIZE 20
typedef struct BeesProps {
    u32   BOXDETECTWOLF;
    u32   BOXFLY;
    u32   FIRSTHIVE;
    u32   MOTHERHIVE;
    u32   SECONDHIVE;
} BeesProps;

// class "HoneyPot" (id 108), PROPSIZE 4
typedef struct HoneyPotProps {
    u32   HIVEMOTHER;
} HoneyPotProps;

// class "CrocodileLevel09" (id 109), PROPSIZE 12
typedef struct CrocodileLevel09Props {
    u32   ACTIVATIONBOX;
    u32   TRAJ;
    u32   WALK;
} CrocodileLevel09Props;

// class "Diamond" (id 110), PROPSIZE 12
typedef struct DiamondProps {
    u32   BOXTELEPORT;
    u32   TRAJ;
    u32   ZONE;
} DiamondProps;

// class "Wheel" (id 111), PROPSIZE 32
typedef struct WheelProps {
    u32   LIFT0;
    u32   LIFT1;
    u32   LIFT2;
    u32   LIFT3;
    u32   MOBIL;
    u32   RAISINGTIME;
    u32   SWITCHANGLE;
    u32   TRANSLATION;
} WheelProps;

// class "WheelDummy" (id 112), PROPSIZE 0
// (no properties)

// class "Gossamer_Boss" (id 113), PROPSIZE 36
typedef struct Gossamer_BossProps {
    u32   BRIDGECAMERA;
    u32   LIGHTBOX1;
    u32   LIGHTBOX2;
    u32   LIGHTBOX3;
    u32   MOVEMENTBOX;
    u32   POINTLIST;
    u32   TIMER1;
    u32   TIMER2;
    u32   TIMER3;
} Gossamer_BossProps;

// class "SuperButton" (id 114), PROPSIZE 8
typedef struct SuperButtonProps {
    u32   NOT;
    u32   OUT;
} SuperButtonProps;

// class "RollingCarpet" (id 115), PROPSIZE 32
typedef struct RollingCarpetProps {
    u32   CAMERA;
    u32   MOVING;
    u32   RATIO;
    u32   TARGET;
    u32   TARGET2;
    u32   TARGET3;
    u32   TARGET4;
    u32   TARGET5;
} RollingCarpetProps;

// class "Hive" (id 116), PROPSIZE 12
typedef struct HiveProps {
    u32   CAMERA;
    u32   MOTHER;
    u32   TIME;
} HiveProps;

// class "Piranhas" (id 117), PROPSIZE 0
// (no properties)

// class "RCarpetMobile" (id 118), PROPSIZE 8
typedef struct RCarpetMobileProps {
    u32   SPEEDRATIO;
    u32   TRAJECTORY;
} RCarpetMobileProps;

// class "CrumblyGround" (id 119), PROPSIZE 0
// (no properties)

// class "Ghost" (id 120), PROPSIZE 8
typedef struct GhostProps {
    u32   ACTIVATIONBOX;
    u32   MASTER;
} GhostProps;

// class "SaladRod" (id 121), PROPSIZE 0
// (no properties)

// class "CrocodileLevel11" (id 122), PROPSIZE 40
typedef struct CrocodileLevel11Props {
    u32   ACTIVATIONBOX;
    u32   BOXES;
    u32   CAMERA;
    u32   CINE;
    u32   CINEBOX;
    u32   CINEFLAG;
    u32   CINESHEEPBOX;
    u32   CINETEXT;
    u32   DAFFY;
    u32   REMAINDEADATRESET;
} CrocodileLevel11Props;

// class "Monolithe" (id 123), PROPSIZE 8
typedef struct MonolitheProps {
    u32   CAMERA;
    u32   SHEEPBOX;
} MonolitheProps;

// class "LightSpot" (id 124), PROPSIZE 24
typedef struct LightSpotProps {
    u32   AUTHORIZEDBOX;
    u32   COLOR;
    u32   NBHEIGHTSEGS;
    u32   NUMSIDES;
    u32   RADIUS;
    u32   TRAJECTORY;
} LightSpotProps;

// class "GhostCostume" (id 125), PROPSIZE 0
// (no properties)

// class "Jail" (id 126), PROPSIZE 20
typedef struct JailProps {
    u32   BUTTONHIGH;
    u32   BUTTONLOW;
    u32   BUTTONMIDDLE;
    u32   HEIGHT;
    u32   TRAJECTORY;
} JailProps;

// class "DancingGhost" (id 127), PROPSIZE 12
typedef struct DancingGhostProps {
    u32   CHATBOX;
    u32   PLAYTAMBOURINE;
    u32   TRAJECTORY;
} DancingGhostProps;

// class "Battery" (id 128), PROPSIZE 8
typedef struct BatteryProps {
    u32   ACTIVATIONBOX;
    u32   MASTER;
} BatteryProps;

// class "Key" (id 129), PROPSIZE 0
// (no properties)

// class "PrayingGhost" (id 130), PROPSIZE 56
typedef struct PrayingGhostProps {
    u32   CAMERARESTRICTION1;
    u32   CAMERARESTRICTION2;
    u32   CAMERARESTRICTION3;
    u32   CHECKPOINT;
    u32   CHECKPOINTJAIL;
    u32   FALLINGGATE;
    u32   FALLINGGATEDETEC;
    u32   SHEEP;
    u32   SHEEPBOX;
    u32   STARTBOX;
    u32   TIMEORANGE;
    u32   TRAJECTORY;
    u32   WOLFINVISIBLE1;
    u32   WOLFNOTVISIBLEMONO;
} PrayingGhostProps;

// class "Hoover" (id 131), PROPSIZE 4
typedef struct HooverProps {
    u32   ACTIVATIONBOX;
} HooverProps;

// class "InflatableSheep" (id 132), PROPSIZE 0
// (no properties)

// class "TrafficJams" (id 133), PROPSIZE 20
typedef struct TrafficJamsProps {
    u32   ARRIVINGTIME;
    u32   BOXSTATION;
    u32   BREAKTIME;
    u32   NEXTJAMS;
    u32   TRAINSTATION;
} TrafficJamsProps;

// class "Chronometer" (id 134), PROPSIZE 0
// (no properties)

// class "Train" (id 135), PROPSIZE 24
typedef struct TrainProps {
    u32   DOCKA;
    u32   DOCKB;
    u32   FIRST;
    u32   JUMPINSHEEPBOX;
    u32   OFFTRAINBOX;
    u32   SAFERECALBOX;
} TrainProps;

// class "TrainStation" (id 136), PROPSIZE 24
typedef struct TrainStationProps {
    u32   BOX;
    u32   CAM;
    u32   NEXT;
    u32   SPEEDPERCENT;
    u32   TRAJ;
    u32   WAIT;
} TrainStationProps;

// class "AmbientSoundManager" (id 137), PROPSIZE 32
typedef struct AmbientSoundManagerProps {
    u32   FLAGS;
    u32   MAXDIST;
    u32   REPEATTIMEMAXMS;
    u32   REPEATTIMEMINMS;
    u32   SOUNDBOX;
    u32   SOUNDID;
    u32   SOUNDMAXVOLBOX;
    u32   VOLUME;
} AmbientSoundManagerProps;

// class "GhostHalo" (id 138), PROPSIZE 0
// (no properties)

// class "BipbipLevel14" (id 139), PROPSIZE 20
typedef struct BipbipLevel14Props {
    u32   BOXSTOPBIPBIP;
    u32   GOTOTRAJECTORY;
    u32   RETURNTRAJECTORY;
    u32   TIMEWAIT;
    u32   TRAINSTATION;
} BipbipLevel14Props;

// class "FallingGate2" (id 140), PROPSIZE 20
typedef struct FallingGate2Props {
    u32   _2BUTTON;
    u32   CAMERA;
    u32   CLOSED;
    u32   FALLINGTIME;
    u32   ONLYONEACTIVATION;
} FallingGate2Props;

// class "GoldenCoins" (id 141), PROPSIZE 0
// (no properties)

// class "Sam_Pirate" (id 142), PROPSIZE 36
typedef struct Sam_PirateProps {
    u32   BOX_FORBIDDEN;
    u32   BOX_NEAR_JAIL;
    u32   BRIDGE_BOX;
    u32   JAIL_BOX;
    u32   NBER_OF_COIN;
    u32   TEXT_IN_BOAT1;
    u32   TEXT_IN_BOAT2;
    u32   TEXT_IN_JAIL;
    u32   TRAJ_BOAT;
} Sam_PirateProps;

// class "FloatingBox" (id 143), PROPSIZE 4
typedef struct FloatingBoxProps {
    u32   CONTAINS;
} FloatingBoxProps;

// class "InstantHoover" (id 144), PROPSIZE 20
typedef struct InstantHooverProps {
    u32   ACTIVEBOX;
    u32   BOXPOSITION;
    u32   HORRANGE;
    u32   OPENTIME;
    u32   VERTRANGE;
} InstantHooverProps;

// class "InstantMartian" (id 145), PROPSIZE 44
typedef struct InstantMartianProps {
    u32   BOX;
    u32   BOX2;
    u32   BOX3;
    u32   BOX4;
    u32   BOX5;
    u32   CHECKPOINTBOX;
    u32   COLFORTRAJ;
    u32   DETECTBOX;
    u32   MARTIENBEHAVIOR;
    u32   TRAJECTORY;
    u32   TRAJECTORY2;
} InstantMartianProps;

// class "CanonSheep" (id 146), PROPSIZE 48
typedef struct CanonSheepProps {
    u32   ACTIVATIONBOX;
    u32   CINE;
    u32   CINEFLAG;
    u32   CINETEXT;
    u32   DETECTBOX;
    u32   HORZLIMIT;
    u32   LAUNCHPOINT1;
    u32   LAUNCHPOINT2;
    u32   LAUNCHPOINT3;
    u32   LOADBOX;
    u32   VERTLIMIT;
    u32   VOIDCAMERA;
} CanonSheepProps;

// class "CannonBall2" (id 147), PROPSIZE 0
// (no properties)

// class "SceneSheepPanel" (id 148), PROPSIZE 12
typedef struct SceneSheepPanelProps {
    u32   ID_CAMERA;
    u32   ID_CINE;
    u32   ID_SHEEPBOX;
} SceneSheepPanelProps;

// class "LazerRobot" (id 149), PROPSIZE 8
typedef struct LazerRobotProps {
    u32   SPEED;
    u32   TRAJ;
} LazerRobotProps;

// class "Laser" (id 150), PROPSIZE 16
typedef struct LaserProps {
    u32   ACTIVATIONDELAY;
    u32   BOX;
    u32   INACTIVE;
    u32   TYPE;
} LaserProps;

// class "Case" (id 151), PROPSIZE 0
// (no properties)

// class "Scene_Wheel" (id 152), PROPSIZE 0
// (no properties)

// class "Scene_Arrow" (id 153), PROPSIZE 0
// (no properties)

// class "Scene_Screen" (id 154), PROPSIZE 0
// (no properties)

// class "DaffyScene" (id 155), PROPSIZE 116
typedef struct DaffySceneProps {
    u32   CAMCOUNTER;
    u32   CAMLEVEL;
    u32   CAMSAVE;
    u32   CAMSPEAK1;
    u32   CAMSPEAK2;
    u32   CAMSPEAK3;
    u32   CAMSPEAK4;
    u32   CAMSPEAK5;
    u32   CAMTIMECLOCK;
    u32   CINE;
    u32   CINEFLAG;
    u32   CINETEXT;
    u32   CROWD;
    u32   MAINCAMERA;
    u32   TEXTCOUNTER;
    u32   TEXTFAILINFIRSTLVL;
    u32   TEXTLEVEL;
    u32   TEXTNEWCONTESTANT;
    u32   TEXTNOTIMEKEEPER;
    u32   TEXTPHONE;
    u32   TEXTQUITTER;
    u32   TEXTSAVE;
    u32   TEXTSUPPORT;
    u32   TEXTTIMECLOCK;
    u32   TEXTVICTORY1;
    u32   TEXTVICTORY2;
    u32   TEXTVICTORY3;
    u32   TEXTVICTORY4;
    u32   TEXTVICTORY5;
} DaffySceneProps;

// class "AutomaticDoor" (id 156), PROPSIZE 4
typedef struct AutomaticDoorProps {
    u32   SENSIBLEBOXES;
} AutomaticDoorProps;

// class "Resizer" (id 157), PROPSIZE 16
typedef struct ResizerProps {
    u32   _EXIT;
    u32   _MAXIMIZE;
    u32   ACTIVATIONBOX;
    u32   EXIT;
} ResizerProps;

// class "Bat" (id 158), PROPSIZE 0
// (no properties)

// class "Firefly" (id 159), PROPSIZE 0
// (no properties)

// class "Bird" (id 160), PROPSIZE 4
typedef struct BirdProps {
    u32   APPEARBOX;
} BirdProps;

// class "Butterfly" (id 161), PROPSIZE 8
typedef struct ButterflyProps {
    u32   _4BUTTERFLY;
    u32   APPEARBOX;
} ButterflyProps;

// class "Fish" (id 162), PROPSIZE 4
typedef struct FishProps {
    u32   DISAPPEAR_DIST;
} FishProps;

// class "Pipe2" (id 163), PROPSIZE 8
typedef struct Pipe2Props {
    u32   BOXACT1;
    u32   BOXACTI2;
} Pipe2Props;

// class "Lava" (id 164), PROPSIZE 4
typedef struct LavaProps {
    u32   RANGE;
} LavaProps;

// class "SignPostAnimated" (id 165), PROPSIZE 12
typedef struct SignPostAnimatedProps {
    u32   DISTANCE;
    u32   HIDDEN;
    u32   INDEXTEXT;
} SignPostAnimatedProps;

// class "Volcano" (id 166), PROPSIZE 12
typedef struct VolcanoProps {
    u32   TRAJ1;
    u32   UTURN;
    u32   VERTICAL;
} VolcanoProps;

// class "DancingGhostManager" (id 167), PROPSIZE 96
typedef struct DancingGhostManagerProps {
    u32   BOX_COSTUM;
    u32   BOX_EXT;
    u32   BOX_INT;
    u32   BOX_REMOVE;
    u32   BOX_SAF;
    u32   BOXSTEP1;
    u32   BOXSTEP2;
    u32   BOXSTEP3;
    u32   BOXSTEP4;
    u32   CINE;
    u32   CINEBOX;
    u32   CINEFLAGS;
    u32   CINETEXT;
    u32   CONGRATULID;
    u32   DANCECAM1;
    u32   DANCECAM2;
    u32   DANCECAM3;
    u32   DANCECAM4;
    u32   EXPLAINBUTID;
    u32   EXPLAINBUTSYNCID;
    u32   EXPLAINPATHID;
    u32   EXPLAINSYNCID;
    u32   SEQID;
    u32   STEPCLEAREDID;
} DancingGhostManagerProps;

// class "DoorsMaster" (id 168), PROPSIZE 0
// (no properties)

// class "DoorWorld" (id 169), PROPSIZE 8
typedef struct DoorWorldProps {
    u32   CAMERAOPEN;
    u32   IDDOORLEVEL;
} DoorWorldProps;

// class "DoorLevel" (id 170), PROPSIZE 8
typedef struct DoorLevelProps {
    u32   IDBOXBACK;
    u32   LEVELNUMBER;
} DoorLevelProps;

// class "FireBall" (id 171), PROPSIZE 0
// (no properties)

// class "Twig" (id 172), PROPSIZE 4
typedef struct TwigProps {
    u32   DISAPPEAR_DIST;
} TwigProps;

// class "GeyserManger" (id 173), PROPSIZE 68
typedef struct GeyserMangerProps {
    u32   BOXDETECT;
    u32   GEYSER01;
    u32   GEYSER02;
    u32   GEYSER03;
    u32   GEYSER04;
    u32   GEYSER05;
    u32   GEYSER06;
    u32   GEYSER07;
    u32   GEYSER08;
    u32   GEYSER09;
    u32   GEYSER10;
    u32   GEYSER11;
    u32   GEYSER12;
    u32   GEYSER13;
    u32   GEYSER14;
    u32   GEYSER15;
    u32   GEYSER16;
} GeyserMangerProps;

// class "SignTips" (id 174), PROPSIZE 24
typedef struct SignTipsProps {
    u32   CAMERA;
    u32   CHECKPOINT;
    u32   DIST;
    u32   INDEXTEXT;
    u32   NBREBIRTH;
    u32   TRAJBOXES;
} SignTipsProps;

// class "MirrorManager" (id 175), PROPSIZE 48
typedef struct MirrorManagerProps {
    u32   COLORINTENSITY;
    u32   GROUNDALTITUDE;
    u32   ID_OBJECT01;
    u32   ID_OBJECT02;
    u32   ID_OBJECT03;
    u32   ID_OBJECT04;
    u32   ID_OBJECT05;
    u32   ID_OBJECT06;
    u32   ID_OBJECT07;
    u32   ID_OBJECT08;
    u32   IDBOX;
    u32   MIRRORCOLOR;
} MirrorManagerProps;

// class "CreditsManager" (id 176), PROPSIZE 20
typedef struct CreditsManagerProps {
    u32   COLOR;
    u32   TEXTJUST;
    u32   TEXTNUM;
    u32   TIME_APP;
    u32   TIME_DIS;
} CreditsManagerProps;

// class "WaterMine" (id 177), PROPSIZE 0
// (no properties)

// class "Rook" (id 178), PROPSIZE 24
typedef struct RookProps {
    u32   BOXDETECT;
    u32   CANNON;
    u32   CANNONBALL;
    u32   DISTDETECT;
    u32   SPEED;
    u32   TRAJECTORY;
} RookProps;

// class "SfxCineManager" (id 179), PROPSIZE 40
typedef struct SfxCineManagerProps {
    u32   FINALSIZE;
    u32   LIFETIME;
    u32   MOVESPEED;
    u32   NBSFX;
    u32   POSX;
    u32   POSY;
    u32   POSZ;
    u32   SIZE;
    u32   TARGET;
    u32   TYPESFX;
} SfxCineManagerProps;

// class "CineMatrixMode" (id 180), PROPSIZE 4
typedef struct CineMatrixModeProps {
    u32   BOX;
} CineMatrixModeProps;

// class "CrumblyPlat" (id 181), PROPSIZE 0
// (no properties)

// class "GossamerOnde" (id 182), PROPSIZE 0
// (no properties)

// class "FacingCamera" (id 183), PROPSIZE 0
// (no properties)

// class "ObjectManager" (id 184), PROPSIZE 88
typedef struct ObjectManagerProps {
    u32   ACTION;
    u32   MANAGEINBOXES;
    u32   OBJECT01;
    u32   OBJECT02;
    u32   OBJECT03;
    u32   OBJECT04;
    u32   OBJECT05;
    u32   OBJECT06;
    u32   OBJECT07;
    u32   OBJECT08;
    u32   OBJECT09;
    u32   OBJECT10;
    u32   OBJECT11;
    u32   OBJECT12;
    u32   OBJECT13;
    u32   OBJECT14;
    u32   OBJECT15;
    u32   OBJECT16;
    u32   OBJECT17;
    u32   OBJECT18;
    u32   OBJECT19;
    u32   OBJECT20;
} ObjectManagerProps;

// class "HiddenButton" (id 185), PROPSIZE 12
typedef struct HiddenButtonProps {
    u32   TARGET;
    u32   TARGET2;
    u32   TYPE;
} HiddenButtonProps;

// class "WaterGeyser" (id 186), PROPSIZE 12
typedef struct WaterGeyserProps {
    u32   BOXDECTECT;
    u32   GEYSEROUT;
    u32   TIMEINHOLE;
} WaterGeyserProps;

// class "DaffyWheel" (id 187), PROPSIZE 8
typedef struct DaffyWheelProps {
    u32   IDCAMBEGIN;
    u32   IDCAMERA;
} DaffyWheelProps;

// class "Torch" (id 188), PROPSIZE 0
// (no properties)

// class "Sail" (id 189), PROPSIZE 0
// (no properties)

// class "BonusManager" (id 190), PROPSIZE 4
typedef struct BonusManagerProps {
    u32   IDCAMERA;
} BonusManagerProps;

// class "Leaf" (id 191), PROPSIZE 4
typedef struct LeafProps {
    u32   RANGE;
} LeafProps;

// class "BlackHole" (id 192), PROPSIZE 0
// (no properties)

// class "Marvin" (id 193), PROPSIZE 32
typedef struct MarvinProps {
    u32   CINE;
    u32   CINEBOX;
    u32   CINEFLAGS;
    u32   CINETEXT;
    u32   MOCKERYID1;
    u32   MOCKERYID2;
    u32   SUCCESSID1;
    u32   SUCCESSID2;
} MarvinProps;

// class "InstantSocket" (id 194), PROPSIZE 0
// (no properties)

// class "Crowd" (id 195), PROPSIZE 0
// (no properties)

// class "CameraManager2" (id 196), PROPSIZE 12
typedef struct CameraManager2Props {
    u32   BOXES;
    u32   CAMERA;
    u32   FLAGS;
} CameraManager2Props;

#endif
