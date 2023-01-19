/*
 * dgserver.h
 *
 *  Created on: 30 марта 2015 г.
 *      Author: Usach
 */

#ifndef DGSERVER_H_
#define DGSERVER_H_

#include "../server/server.h"
#include "../qtservice/src/qtservice.h"
#include "../qtsingleapplication/src/qtsinglecoreapplication.h"

//#include <QCoreApplication>

class dgserver : public QtService<QtSingleCoreApplication> {

public:
	dgserver(int argc, char **argv);

   ~dgserver();

   bool isExit(){return exitBit;}

protected:
    void start();
    void stop();
    void pause();
    void resume();
 //   void processCommand(int code);

private:
    void initDG();
    int testApp();

    server*   s;
    logFile*  LogF;

    bool      isService;
    bool      autoRun;
    bool      exitBit;

};

#endif /* DGSERVER_H_ */
