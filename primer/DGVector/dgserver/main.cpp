#include <QtCore>
#include "dgserver.h"

int main(int argc, char *argv[])
{

#if defined(Q_OS_WIN)
	QStringList paths = QCoreApplication::libraryPaths();
	paths.append(".");
	paths.append("imageformats");
	paths.append("platforms");
	paths.append("sqldrivers");
	QCoreApplication::setLibraryPaths(paths);
#endif
	dgserver service(argc,argv);
    if(service.isExit()) return 0;
    return service.exec();

}
