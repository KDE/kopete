#include <kdialog.h>
#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './avdeviceconfig_videoconfig.ui'
**
** Created: Dom Mai 8 09:11:50 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.4   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "avdeviceconfig_videoconfig.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <kcombobox.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

static const char* const img0_avdeviceconfig_videoconfig[] = { 
"22 22 233 2",
".I c #000103",
"#V c #010002",
"a4 c #010102",
"br c #010103",
"#Z c #010105",
"ai c #010202",
"a# c #010203",
".H c #010204",
"#D c #020104",
"aB c #020105",
"#J c #020106",
"#5 c #020107",
"aM c #020202",
"bq c #020203",
".G c #020204",
"b. c #020205",
"## c #020206",
"aW c #020207",
".J c #020209",
".K c #02020a",
"bs c #020304",
"#Q c #020305",
".V c #020306",
".W c #020307",
"aG c #020407",
".U c #020408",
"bt c #030103",
".F c #030203",
"bj c #030205",
".P c #030209",
"#u c #030303",
".2 c #030304",
"aj c #030306",
"a9 c #030307",
"#6 c #030308",
"#a c #030403",
"av c #030406",
"bk c #030602",
"a3 c #030604",
"aN c #030606",
"bv c #030800",
"aQ c #040205",
"aP c #040302",
"ax c #040405",
"aw c #040407",
"aL c #040505",
"#P c #040506",
"#R c #040605",
"ah c #040608",
"#z c #04060a",
"#4 c #040703",
"au c #04070b",
".3 c #050203",
"aI c #050706",
"bu c #080c03",
".L c #080e0a",
"#q c #090b05",
"bp c #090b06",
"#i c #090c06",
"a5 c #090d04",
".E c #0a0b09",
"aV c #0b0c0a",
"#U c #111705",
"#K c #12150b",
"bi c #12170a",
"#p c #12180a",
"b# c #121909",
".T c #12190a",
".Q c #131608",
"#j c #131709",
"aA c #13170a",
"as c #13180a",
"bh c #131906",
".4 c #14170b",
"#. c #14180b",
"#Y c #151b06",
".s c #151b07",
"bB c #161a08",
"bA c #161a0a",
"ao c #161b08",
"aa c #161b09",
".t c #161c07",
"ar c #283210",
".r c #29320f",
"#I c #29330f",
".u c #293310",
"bz c #2b330e",
"#W c #2b330f",
"aC c #2b3411",
"bC c #2c3411",
".R c #2c3512",
"#0 c #2c3514",
"ae c #2c3612",
".S c #2c3613",
"a. c #2d3611",
"an c #2d3711",
"#A c #323f11",
"aO c #373f1b",
"bd c #374114",
"bc c #383f1c",
".O c #3f4c12",
".X c #414e14",
"bg c #424c15",
"aF c #566816",
".M c #56691a",
"#t c #57661c",
".q c #57671c",
".v c #576814",
"by c #58651d",
"#E c #58651e",
"aR c #586817",
".D c #58681b",
"a8 c #59671a",
"bD c #596817",
"#b c #59681a",
".1 c #596919",
"a2 c #5a6d1d",
"ay c #5a6e1a",
"bw c #5a6f19",
"ag c #5b6c1f",
"#y c #5b6d1f",
"#O c #5c6e1b",
"#3 c #5c6e1d",
"bE c #5c701b",
"bl c #5c711b",
"#7 c #5d6b1d",
"aJ c #5e6b1e",
"ak c #5e6b20",
"aK c #5f6c21",
".9 c #6c8220",
"ba c #6d821d",
"#C c #6d821e",
".5 c #6e8018",
"#S c #6e871b",
"aZ c #6e871c",
"#m c #6e8815",
"at c #6e8916",
"#v c #6f8119",
"aX c #8ba823",
"#o c #8ca823",
"#k c #8da824",
"a6 c #90ae19",
".w c #90ae1f",
".p c #92ad1b",
"#h c #92ad1c",
"bx c #93ab20",
"#r c #93ab21",
"aU c #93ad20",
"#B c #9aba1b",
"aY c #9abb1a",
"#n c #9bbc22",
"aH c #9cbc22",
".8 c #9ebf1e",
"az c #9fbc21",
"#T c #9fbd20",
"bb c #9fbe1d",
"a1 c #9fc315",
"a0 c #9fc318",
"al c #9fc412",
".C c #a0c117",
".N c #a0c119",
"#N c #a0c311",
"am c #a0c317",
"#8 c #a0c319",
".B c #a0c610",
".Y c #a1c115",
"ad c #a1c11d",
"#c c #a1c11f",
".i c #a1c218",
".j c #a1c21e",
".g c #a1c412",
".l c #a1c413",
"bM c #a1c414",
"aT c #a1c416",
".n c #a1c511",
".x c #a1c512",
"aE c #a1c513",
".z c #a1c514",
".y c #a1c517",
".m c #a1c60f",
".A c #a1c611",
".6 c #a2bc1e",
"ab c #a2c01d",
"#X c #a2c312",
".k c #a2c320",
".o c #a2c411",
".0 c #a2c412",
"#G c #a2c413",
"#f c #a2c414",
"#2 c #a2c415",
"bK c #a2c416",
"#H c #a2c417",
"af c #a2c41b",
"#w c #a2c50d",
"bL c #a2c50e",
".e c #a2c50f",
".# c #a2c510",
"Qt c #a2c511",
".a c #a2c512",
".b c #a2c513",
".c c #a2c514",
".d c #a2c515",
"#9 c #a2c516",
"#l c #a2c51b",
"#L c #a2c60c",
"#M c #a2c60e",
"#e c #a2c60f",
".f c #a2c610",
".Z c #a2c611",
"ac c #a2c612",
"#x c #a2c70c",
"bf c #a3be1a",
"bo c #a3c117",
"#F c #a3c312",
"bI c #a3c313",
"#1 c #a3c316",
"aq c #a3c31d",
"bm c #a3c412",
".h c #a3c417",
"bF c #a3c41c",
"#d c #a3c517",
"#g c #a3c610",
"bG c #a3c611",
"ap c #a4c220",
".7 c #a4c316",
"bJ c #a4c31a",
"aD c #a4c31b",
"bn c #a4c410",
"a7 c #a4c412",
"bH c #a4c511",
"#s c #a4c512",
"aS c #a4c513",
"be c #a5c418",
"QtQtQtQtQtQtQt.#Qt.a.b.c.b.#QtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQt.d.c.a.aQt.e.aQtQtQtQtQtQtQtQt",
"QtQtQtQtQtQtQt.f.g.h.i.j.k.lQtQtQtQtQtQtQtQt",
"QtQtQt.m.n.c.o.p.q.r.s.t.u.v.w.x.y.z.b.aQtQt",
"QtQtQt.A.B.C.D.E.F.G.H.I.J.K.L.M.N.c.#.#QtQt",
"QtQtQt.c.C.O.F.P.K.Q.R.S.T.U.V.W.X.Y.ZQtQtQt",
"QtQtQt.0.1.2.3.4.5.6.7.d.8.9#.###a#b#c#d#e.#",
".e#f#g#h#i.F#j#kQt.e.f#l#m#n#o#p.H#q#r#s.zQt",
"Qt.b.0#t#u.J#v#w.l#x.a#y#z#A#B#C.V#D#E#F.z.a",
"#G.0#H#I#J#K.6#L#M#N#O#P#Q#R#S#T#U#V#W#X.x.a",
".b.a.i#Y#Z#0#1.##2#3#4#5#6#7#8#9a.a#aaab.nQt",
".bacad.t.Iae.zafagah#Jaiajakalaman.Haoap.nQt",
".b.aaqar.Kas.8atauavawaj#5axayazaAaBaCaDaE.a",
"#e#2.eaF.KaG.9aH#AaIaJaKaLaMaNaOaPaQaRaSaTQt",
".#.a.#aUaVaW.4aXaYaZa0a1a2a3a4a4a4a5a6.a.#.a",
"QtQtQta7a8a9b.b#babb.baEazbca4aM#ubd#H#G#MQt",
"QtQtQtbebfbgaL.H.Wbha.a.bibja4#uaMbkbl.d.#.#",
"QtQtQtbmbnbo.Dbpbqbrbs.H.Gbtbubdbkbvbw#f.#Qt",
"QtQtQt.a.##fadbxbybzbAbBbCbDa6#HbEbwbF#GQtQt",
"QtQtQt.b#2.bbGbHbI#FabapbJbm.a#G#2bK#2.bQtQt",
"QtQtQtbL.##f.ebM.z.x.n.n.xaT.#.e.#.a#2.b.#.e",
"QtQtQtQtQt.aQtQt.a.aQtQt.aQtQtQt.#Qt.aQt.e.#"};


/*
 *  Constructs a AVDeviceConfig_VideoDevice as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
AVDeviceConfig_VideoDevice::AVDeviceConfig_VideoDevice( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl ),
      image0( (const char **) img0_avdeviceconfig_videoconfig )
{
    if ( !name )
	setName( "AVDeviceConfig_VideoDevice" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
    AVDeviceConfig_VideoDeviceLayout = new QGridLayout( this, 1, 1, 11, 6, "AVDeviceConfig_VideoDeviceLayout"); 

    layout21 = new QVBoxLayout( 0, 0, 6, "layout21"); 

    layout19 = new QHBoxLayout( 0, 0, 6, "layout19"); 
    imageLeftSpacer = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout19->addItem( imageLeftSpacer );

    imagePixMap = new QLabel( this, "imagePixMap" );
    imagePixMap->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, imagePixMap->sizePolicy().hasHeightForWidth() ) );
    imagePixMap->setMinimumSize( QSize( 320, 240 ) );
    imagePixMap->setPixmap( image0 );
    imagePixMap->setScaledContents( TRUE );
    layout19->addWidget( imagePixMap );
    imageRightSpacer = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout19->addItem( imageRightSpacer );
    layout21->addLayout( layout19 );

    buttonGroup7 = new QButtonGroup( this, "buttonGroup7" );
    buttonGroup7->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, 0, 0, buttonGroup7->sizePolicy().hasHeightForWidth() ) );
    buttonGroup7->setColumnLayout(0, Qt::Vertical );
    buttonGroup7->layout()->setSpacing( 6 );
    buttonGroup7->layout()->setMargin( 11 );
    buttonGroup7Layout = new QGridLayout( buttonGroup7->layout() );
    buttonGroup7Layout->setAlignment( Qt::AlignTop );

    videodevice_selection_layout = new QHBoxLayout( 0, 0, 6, "videodevice_selection_layout"); 

    videodevice_selection_labels = new QVBoxLayout( 0, 0, 6, "videodevice_selection_labels"); 

    deviceLabel = new QLabel( buttonGroup7, "deviceLabel" );
    videodevice_selection_labels->addWidget( deviceLabel );

    inputLabel = new QLabel( buttonGroup7, "inputLabel" );
    videodevice_selection_labels->addWidget( inputLabel );

    standardLabel = new QLabel( buttonGroup7, "standardLabel" );
    videodevice_selection_labels->addWidget( standardLabel );
    videodevice_selection_layout->addLayout( videodevice_selection_labels );

    videodevice_selection_combos = new QVBoxLayout( 0, 0, 6, "videodevice_selection_combos"); 

    devicekComboBox = new KComboBox( FALSE, buttonGroup7, "devicekComboBox" );
    devicekComboBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, devicekComboBox->sizePolicy().hasHeightForWidth() ) );
    videodevice_selection_combos->addWidget( devicekComboBox );

    inputkComboBox = new KComboBox( FALSE, buttonGroup7, "inputkComboBox" );
    inputkComboBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, inputkComboBox->sizePolicy().hasHeightForWidth() ) );
    videodevice_selection_combos->addWidget( inputkComboBox );

    normkComboBox = new KComboBox( FALSE, buttonGroup7, "normkComboBox" );
    normkComboBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, normkComboBox->sizePolicy().hasHeightForWidth() ) );
    videodevice_selection_combos->addWidget( normkComboBox );
    videodevice_selection_layout->addLayout( videodevice_selection_combos );

    buttonGroup7Layout->addLayout( videodevice_selection_layout, 0, 0 );
    layout21->addWidget( buttonGroup7 );

    buttonGroup12 = new QButtonGroup( this, "buttonGroup12" );
    buttonGroup12->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, 0, 0, buttonGroup12->sizePolicy().hasHeightForWidth() ) );
    buttonGroup12->setColumnLayout(0, Qt::Vertical );
    buttonGroup12->layout()->setSpacing( 6 );
    buttonGroup12->layout()->setMargin( 11 );
    buttonGroup12Layout = new QGridLayout( buttonGroup12->layout() );
    buttonGroup12Layout->setAlignment( Qt::AlignTop );

    layout25 = new QVBoxLayout( 0, 0, 6, "layout25"); 

    layout24 = new QHBoxLayout( 0, 0, 6, "layout24"); 

    layout22 = new QVBoxLayout( 0, 0, 6, "layout22"); 

    brightLabel = new QLabel( buttonGroup12, "brightLabel" );
    layout22->addWidget( brightLabel );

    contrastLabel = new QLabel( buttonGroup12, "contrastLabel" );
    layout22->addWidget( contrastLabel );

    saturationLabel = new QLabel( buttonGroup12, "saturationLabel" );
    layout22->addWidget( saturationLabel );

    hueLabel = new QLabel( buttonGroup12, "hueLabel" );
    layout22->addWidget( hueLabel );
    layout24->addLayout( layout22 );

    layout23 = new QVBoxLayout( 0, 0, 6, "layout23"); 

    brightSlider = new QSlider( buttonGroup12, "brightSlider" );
    brightSlider->setOrientation( QSlider::Horizontal );
    layout23->addWidget( brightSlider );

    contrastSlider = new QSlider( buttonGroup12, "contrastSlider" );
    contrastSlider->setOrientation( QSlider::Horizontal );
    layout23->addWidget( contrastSlider );

    saturationSlider = new QSlider( buttonGroup12, "saturationSlider" );
    saturationSlider->setOrientation( QSlider::Horizontal );
    layout23->addWidget( saturationSlider );

    hueSlider = new QSlider( buttonGroup12, "hueSlider" );
    hueSlider->setOrientation( QSlider::Horizontal );
    layout23->addWidget( hueSlider );
    layout24->addLayout( layout23 );
    layout25->addLayout( layout24 );

    imageAutoAdjustBrightContrast = new QCheckBox( buttonGroup12, "imageAutoAdjustBrightContrast" );
    layout25->addWidget( imageAutoAdjustBrightContrast );

    buttonGroup12Layout->addLayout( layout25, 0, 0 );
    layout21->addWidget( buttonGroup12 );

    AVDeviceConfig_VideoDeviceLayout->addLayout( layout21, 0, 0 );
    languageChange();
    resize( QSize(469, 575).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

/*
 *  Destroys the object and frees any allocated resources
 */
AVDeviceConfig_VideoDevice::~AVDeviceConfig_VideoDevice()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void AVDeviceConfig_VideoDevice::languageChange()
{
    setCaption( tr2i18n( "Video" ) );
    buttonGroup7->setTitle( tr2i18n( "&Video Device Configuration" ) );
    deviceLabel->setText( tr2i18n( "Device:" ) );
    inputLabel->setText( tr2i18n( "Input:" ) );
    standardLabel->setText( tr2i18n( "Standard:" ) );
    buttonGroup12->setTitle( tr2i18n( "&Image Adjustment" ) );
    brightLabel->setText( tr2i18n( "Bright" ) );
    contrastLabel->setText( tr2i18n( "Contrast" ) );
    saturationLabel->setText( tr2i18n( "Saturation" ) );
    hueLabel->setText( tr2i18n( "Hue" ) );
    imageAutoAdjustBrightContrast->setText( tr2i18n( "Au&tomatic bright/contrast adjustment" ) );
}

#include "avdeviceconfig_videoconfig.moc"
