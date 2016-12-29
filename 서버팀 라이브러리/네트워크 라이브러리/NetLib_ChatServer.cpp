#include "StdAfx.h"
#include "NetworkSession.h"
#include "NetService.h"

#include <queue>
#include <utility>

class ChatListener : public WorkListener{
public:
	int workPerformed( Work* work, void* arg ){
		// cast parameter
		SessionOverlapped *pSession = (SessionOverlapped *) work;
		CLumpData *msgBuf = (CLumpData *) arg;
		
		std::pair<char *, int> *chat = new std::pair<char *, int>;

		chat->first = msgBuf->GetCursor();
		chat->second = strlen(chat->first);

		// push message to queue
		chatQueue.push( chat );		
		return 0;
	}
	int WorkListener::specificPerformed(Work *,void *){
		return 0;
	}

	int orderPerformed( Work* work, void* arg ){
		NetServiceOrder *pOrder = (NetServiceOrder *) arg;

		// if exists message,
		if( ! this->chatQueue.empty() ){
			// broadcast message
			std::pair<char *, int> *chat = chatQueue.front();
			chatQueue.pop();
			
			pOrder->order_broadcast( chat->first, chat->second );

			delete chat;
		}
		return 0;
	}
	
	ChatListener(size_t type) : WorkListener( type ){}

private:
	std::queue< std::pair<char *, int> *> chatQueue;
};

int main(){
	// make config obj & setting
	NetworkConfig cfg;
	cfg.SetInfomation( 7070, 5, 3000, 2, 10, 100); 
	cfg.SetPollerType( DEF_POLLER_IOCP );

	// make listener
	ChatListener *t = new ChatListener(0);
	
	// start netservice
	NetService::Instance()->startService( t, "chat server", &cfg );
	
	// loop...
	while(1)
		Sleep(1000);
	return 0;
}