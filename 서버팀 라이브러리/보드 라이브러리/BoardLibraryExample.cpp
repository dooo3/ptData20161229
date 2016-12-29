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
		// ������ �濡 ������ ��, �� ��1
		return 0;
	}
	int makeRemoveUser( User* user, char* buffer, size_t size ){
		return 0;
	}
	int makeNewUser( User* user, char* buffer, size_t size ){
		// ������ �濡 ������ ��, �� ��4
		return 0;
	}
	int makeRoomDetail( User* user, char* buffer, size_t size ){
		// ������ �濡 ������ ��, �� ��2
		return 0;
	}
	int makeRoomUserList( byte make_type, User* user, User* another_user, \
		char* buffer, size_t size ){
		// ������ �濡 ������ ��, �� ��3
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
		// ������ �濡 ������ ��, ���� 5
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
		printf("\n��� ���� ���� ��������ϴ�. ���� : %s\n", buffer );
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
	//- �κ� ���ü� �ִ���
	int enterCheck( User* user ){ return 0; }

	//- �κ� ���°� ����
	int makeLobbyInStart( User* user, char* buffer, size_t size ){ return 0; }

	//- �α��ν� �������� ������ ���
	int makeMyInfo( User* user, char* buffer, size_t size ){ return 0; }

	//- �κ񸮽�Ʈ���� ������ ������� ���
	int makeRemoveUser( User* user, char* buffer, size_t size ){ return 0; }

	//- �κ񿡼� ���� ������� ���
	int makeRemoveRoom( Room* room, char* buffer, size_t size ){ return 0; }
	int makeUserList( byte make_type, User* user, \
		char* buffer, size_t size ){ return 0; }

	//- ������ �κ�� ���Դ�
	int enterNewUser( User* user ){ 
		printf("\n����! ���� ������ �κ� ���Խ��ϴ�! : %s\n", user->getID() );

		printf("���� ������ : %d\n", this->users_.size());

		printf("\n==���� ����Ʈ==\n");
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
	// 1. User/Room/Lobby Manager�� �����ϰ� LobbyService�� ����Ѵ�.
	mgameUserManager userManagerObj;
	mgameRoomManager roomManagerObj;
	mgameLobbyManager lobbyManagerObj;

	LobbyService::Instance()->setMgrInstance( &userManagerObj, &roomManagerObj, &lobbyManagerObj );
	
	// 2. LobbyEnter/RoomCreate/RoomEnter/RoomExit�� �����ϰ�, LobbyService�� ����Ѵ�. _Default�� �̸� �����Ǿ� ����
	LobbyEnter_Default lobbyEnterObj( NULL );
	RoomCreate_Default roomCreateObj( NULL );
	RoomEnter_Default roomEnterObj( NULL );
	RoomExit_Default roomExitObj( NULL );
	
	LobbyService::Instance()->setActionInstance( &lobbyEnterObj, &roomCreateObj, &roomEnterObj, &roomExitObj );
	
	// 3. xml ���� ������ ���� �ʱ�ȭ�Ѵ�.
	LobbyService::Instance()->initialize("board_lib_environment.xml");

	// 4. LobbyService���� get..() �޼����� ���� �Ŵ����� ��� ó���Ѵ�.
	LobbyEnter_Default *pLE = (LobbyEnter_Default *)LobbyService::Instance()->getLobbyEnter();
	//RoomCreate_Default *pRC = (RoomCreate_Default *)LobbyService::Instance()->getRoomCreate();
	//RoomEnter_Default *pRE = (RoomEnter_Default *)LobbyService::Instance()->getRoomEnter();

	mgameUser user1;
	user1.setID( "ȫ�浿" );
	user1.setIdent( 1 );
	user1.setPasswd( "1234" );
	pLE->enter(NULL, &user1);

	mgameUser user2;
	user2.setID( "��浿" );
	user2.setIdent( 2 );
	user2.setPasswd( "5432" );
	pLE->enter(NULL, &user2);

	mgameUser user3;
	user3.setID( "�Ѹ�" );
	user3.setIdent( 3 );
	user3.setPasswd( "7777" );
	pLE->enter(NULL, &user3);

	// AutoMakeRoom ��� ����
	// 0. AutoMakeRoom ��ü�� �����.
	AutoMakeRoom AutoMakeRoomUtility;
	
	// 1. ������ ����Ѵ�.
	AutoMakeRoomUtility.register_owner( &user2, "�����¿���\0", strlen("�����¿���")+1 );

	// 2. ���� �̿��� ������ �ִٸ� ����Ѵ�.
	AutoMakeRoomUtility.register_member( &user1 );
	//AutoMakeRoomUtility.register_member( &user2);	// owner�� member�� ����ϸ� �� �ȴ�.
	AutoMakeRoomUtility.register_member( &user3 );

	// 3. ���� �����.	�ʱ�ȭ�ǹǷ� 1~3���� �ݺ��� ����Ͽ� ���� ���� �� �ִ�.
	AutoMakeRoomUtility.make();

	/********************************************************************/
	
	printf("\n������� �� ������ �о�ɴϴ�.\n");
	RoomManager *pRoomManager = LobbyService::Instance()->getRoomManager();

	size_t lastRoomIdx =  pRoomManager->size();
	Room *pRoom = pRoomManager->get( lastRoomIdx );

	printf("��(%s)���� ���� %d���� ������ �ֽ��ϴ�.\n", pRoom->getName(), pRoom->getUserCount() );
	printf("\n������ %s ���̽ʴϴ�.\n", pRoom->getOwner()->getID() );

	
	return 0;
}

