/*
 * maiaCore.cpp
 *
 *  Created on: 13 янв. 2015 г.
 *      Author: Usach
 */

#include "maiaCore.h"

int maiaCore::getKepTime(QHttpHeader* header){
	int timeLife=0;
	if(header->value("Connection").toLower()=="keep-alive"){
		if(header->hasKey("Keep-Alive")){
			QString par=header->value("Keep-Alive").toLower();
			QStringList lpar=par.split(",", QString::SkipEmptyParts);
			for(int i=0;i<lpar.size();i++){
				if(lpar.at(i).contains("timeout",Qt::CaseInsensitive)){
					QStringList tpar=lpar.at(i).split("=", QString::SkipEmptyParts);
					if(tpar.size()>=2){
						timeLife=tpar.at(1).toUInt();
						break;
					}
				}
			}
		}
	}
	return timeLife;

}



int maiaCore::getContLeng(QHttpHeader* header){
	if(header->hasContentLength()) return header->contentLength();
  	else if(header->hasKey("Transfer-Encoding")) {
		if( header->value("Transfer-Encoding").toLower()=="chunked") return -1;
	}
  	return 0;
}



