
#include "dgvector.h"
#include <server.h>
#include <QApplication>
#include <QTranslator>
#include <qtsingleapplication.h>
#include <dgserver_global.h>

QTranslator  translator;

int main(int argc, char *argv[])
{

#if QT_VERSION <= 0x050000
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif

   	bool Show=true;

	//разбор параметров
	for(int i=0;i<argc;i++)
	{
		QString txt(argv[i]);
		txt=txt.toLower();
		if(txt.left(2)=="-d"){
			tunerWin::DebARG<<txt;
			SYSTEM::Debug=true;
			int n=txt.mid(2).toUInt();
			if(n>0 && n<=247){
				SYSTEM::NumDebug=n;
				com485::debugN=n;
			}
		}
		else if(txt.left(2)=="-c")   tunerWin::DebARG<<txt;
		else if(txt.left(2)=="-w")   tunerWin::DebARG<<txt;
		else if(txt.left(2)=="-m")   tunerWin::DebARG<<txt;
		else if(txt.left(2)=="-t")   Show=false;
	}

#if defined(Q_OS_WIN)
	QStringList paths = QCoreApplication::libraryPaths();
	paths.append(".");
	paths.append("imageformats");
	paths.append("platforms");
	paths.append("sqldrivers");
	QCoreApplication::setLibraryPaths(paths);
#endif


	    QtSingleApplication app(argc, argv);
	 	if(app.sendMessage("Wake up dgvector!",1000)) return 0;

	 	translator.load(":/lang/ru/resource/dgvector_ru.qm",".");
	 	app.installTranslator(&translator);


      dgvector w;  //графика
#ifdef _WIN_
	#if QT_VERSION >= 0x050000
      qInstallMessageHandler(logO::logOutput5);
	#else
      qInstallMsgHandler(logO::logOutput);
	#endif
#endif
      if(Show) w.show();

      return app.exec();
}



