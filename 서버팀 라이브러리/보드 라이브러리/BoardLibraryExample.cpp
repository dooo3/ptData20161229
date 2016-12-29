#include "StdAfx.h"

#include "LobbyService.h"

#include "User.h"
#include "Room.h"

#include "UserManager.h"
#include "RoomManager.h"
#include "NotifyManager.h"
#include "LobbyManager.h"

#include "LobbyEnter_Default.h"
#include "RoomCreate_Default.h"
#include "RoomEnter_Default.h"
#include "RoomExit_Default.h"

#include "AutoMakeRoom.h"

class mgameUserManager : public UserManager{
public:
	void removeUser( const size_t ident ){
		this->remove( this->get( ident ) );
	}
	void removeUser( User* user ){
		this->remove( user );
	}
	int broadcast_this_contents( char* data ){
		return 0;
	}
};

class mgameRoom : Room{
public:
	int makeRoomInStart( User* user, char* buffer, size_t size ){
		// 유저가 방에 들어왔을 때, 할 일1
		return 0;
	}
	int makeRemoveUser( User* user, char* buffer, size_t size ){
		return 0;
	}
	int makeNewUser( User* user, char* buffer, size_t size ){
		// 유저가 방에 들어왔을 때, 할 일4
		return 0;
	}
	int makeRoomDetail( User* user, char* buffer, size_t size ){
		// 유저가 방에 들어왔을 때, 할 일2
		return 0;
	}
	int makeRoomUserList( byte make_type, User* user, User* another_user, \
		char* buffer, size_t size ){
		// 유저가 방에 들어왔을 때, 할 일3
		return 0;
	}
	int makeChangeOwner( User* prev_owner, char* buffer, size_t idx, size_t size ){
		return 0;
	}

	int enterCheck( User* user ){
		return 0;
	}
	int exitCheck( User* user, int type ){
		return 0;
	}
	int enterNewUser( User* user ){
		// 유저가 방에 들어왔을 때, 할일 5
		return 0;
	}
	int exitUser( User* user ){
		return 0;
	}		
};


class mgameRoomManager : public RoomManager{
public:
	int createCheck( const char* buffer, User* user ){
		return 0;
	}

	Room* create( const char* buffer, User* user ){
		Room *pRoom = (Room *)new mgameRoom;
		printf("\n모두 들어올 방을 만들었습니다. 방제 : %s\n", buffer );
		pRoom->setName( buffer );
		
		//RoomManager *pRoomManager = LobbyService::Instance()->getRoomManager();

		//pRoomManager->add( pRoom );
		return pRoom;
	}

	int makeRoomList( byte make_type, Room* room, \
		char* buffer, size_t size ){
		return 0;
	}	

	int makeNewRoomInfo( User* user, Room* room, char* buffer, size_t size ){
		return 0;
	}
protected:	
	void destroy( Room* room ){}
};


class mgameLobbyManager : public LobbyManager{
public:	
	//- 로비에 들어올수 있는지
	int enterCheck( User* user ){ return 0; }

	//- 로비에 들어가는것 시작
	int makeLobbyInStart( User* user, char* buffer, size_t size ){ return 0; }

	//- 로그인시 내정보를 보낼때 사용
	int makeMyInfo( User* user, char* buffer, size_t size ){ return 0; }

	//- 로비리스트에서 유저가 사라질때 사용
	int makeRemoveUser( User* user, char* buffer, size_t size ){ return 0; }

	//- 로비에서 방이 사라질때 사용
	int makeRemoveRoom( Room* room, char* buffer, size_t size ){ return 0; }
	int makeUserList( byte make_type, User* user, \
		char* buffer, size_t size ){ return 0; }

	//- 유저가 로비로 들어왔다
	int enterNewUser( User* user ){ 
		printf("\n경축! 드디어 유저가 로비에 들어왔습니다! : %s\n", user->getID() );

		printf("현재 유저수 : %d\n", this->users_.size());

		printf("\n==유저 리스트==\n");
		for(int idx = 0; idx < this->users_.size(); idx++)
			printf("%s\n", this->users_.get_idx( idx )->getID() );
		return 0;
	}

};

class mgameUser : public User{
public:	
	int extern_send( int type, char* data ){
		return 0;
	}
	int send( char* data, size_t size ){
		return 0;
	}
};


int main(){
	// 1. User/Room/Lobby Manager를 구현하고 LobbyService에 등록한다.
	mgameUserManager userManagerObj;
	mgameRoomManager roomManagerObj;
	mgameLobbyManager lobbyManagerObj;

	LobbyService::Instance()->setMgrInstance( &userManagerObj, &roomManagerObj, &lobbyManagerObj );
	
	// 2. LobbyEnter/RoomCreate/RoomEnter/RoomExit를 구현하고, LobbyService에 등록한다. _Default로 미리 구현되어 있음
	LobbyEnter_Default lobbyEnterObj( NULL );
	RoomCreate_Default roomCreateObj( NULL );
	RoomEnter_Default roomEnterObj( NULL );
	RoomExit_Default roomExitObj( NULL );
	
	LobbyService::Instance()->setActionInstance( &lobbyEnterObj, &roomCreateObj, &roomEnterObj, &roomExitObj );
	
	// 3. xml 설정 파일을 통해 초기화한다.
	LobbyService::Instance()->initialize("board_lib_environment.xml");

	// 4. LobbyService에서 get..() 메서드들로 각종 매니저를 얻어 처리한다.
	LobbyEnter_Default *pLE = (LobbyEnter_Default *)LobbyService::Instance()->getLobbyEnter();
	//RoomCreate_Default *pRC = (RoomCreate_Default *)LobbyService::Instance()->getRoomCreate();
	//RoomEnter_Default *pRE = (RoomEnter_Default *)LobbyService::Instance()->getRoomEnter();

	mgameUser user1;
	user1.setID( "홍길동" );
	user1.setIdent( 1 );
	user1.setPasswd( "1234" );
	pLE->enter(NULL, &user1);

	mgameUser user2;
	user2.setID( "김길동" );
	user2.setIdent( 2 );
	user2.setPasswd( "5432" );
	pLE->enter(NULL, &user2);

	mgameUser user3;
	user3.setID( "둘리" );
	user3.setIdent( 3 );
	user3.setPasswd( "7777" );
	pLE->enter(NULL, &user3);

	// AutoMakeRoom 사용 순서
	// 0. AutoMakeRoom 객체를 만든다.
	AutoMakeRoom AutoMakeRoomUtility;
	
	// 1. 방장을 등록한다.
	AutoMakeRoomUtility.register_owner( &user2, "방제는여기\0", strlen("방제는여기")+1 );

	// 2. 방장 이외의 유저가 있다면 등록한다.
	AutoMakeRoomUtility.register_member( &user1 );
	//AutoMakeRoomUtility.register_member( &user2);	// owner는 member로 등록하면 안 된다.
	AutoMakeRoomUtility.register_member( &user3 );

	// 3. 방을 만든다.	초기화되므로 1~3번을 반복해 계속하여 방을 만들 수 있다.
	AutoMakeRoomUtility.make();

	/********************************************************************/
	
	printf("\n만들어진 방 정보를 읽어옵니다.\n");
	RoomManager *pRoomManager = LobbyService::Instance()->getRoomManager();

	size_t lastRoomIdx =  pRoomManager->size();
	Room *pRoom = pRoomManager->get( lastRoomIdx );

	printf("방(%s)에는 현재 %d명의 유저가 있습니다.\n", pRoom->getName(), pRoom->getUserCount() );
	printf("\n방장은 %s 님이십니다.\n", pRoom->getOwner()->getID() );

	
	return 0;
}

