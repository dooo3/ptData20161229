/* ���� ���� ���� :
	1. Base Lib
	2. DB Lib (../lib ���丮�� tinyxml.lib, mysql/ibmysql.lib, mysql/libmysql.dll �߰� �ʿ�)
	3. SQL ���� ���� �� ODBC ����
*/

#include "StdAfx.h"

#include "Work.h"

#include "ODBC_DBSession.h"
#include "ODBC_DBStatement.h"
#include "ODBC_DBResultSet.h"

#include "DBService.h"
#include "DBQueryQ.h"	
#include "DBSessionProxy.h"


/* DBWork */
class DBWork : public Work{
public:
	enum{
		BUFFER_SIZE = 1024
	};

	DBWork( char *pDBName ){
		::ZeroMemory( this->DBName, BUFFER_SIZE );
		strcpy_s( this->DBName, BUFFER_SIZE, pDBName );
	}

	virtual void* resultSetHandler( DBResultSet *pResultSet ) = 0;

	inline char *getDBName(){
		return this->DBName;
	}
	inline void setDBName( char * pDBName ){
		strcpy_s( this->DBName, BUFFER_SIZE, pDBName );
	}

private:
	char DBName[BUFFER_SIZE];

};

/* ODBC_DBWork */
class ODBC_DBWork : public DBWork{
public:
	ODBC_DBWork(char *pDBName) : DBWork( pDBName ){
	}
	void* resultSetHandler( DBResultSet *pResultSet );
private:
};


void* ODBC_DBWork::resultSetHandler( DBResultSet *pResultSet ){
	ODBC_DBResultSet * pODBC_DBResultSet = (ODBC_DBResultSet *) pResultSet;

	char id[ BUFFER_SIZE ];
	char passwd[ BUFFER_SIZE ];

	// ResultSet���� ó���� ���� �б� ���� next()�� �����ؼ� fetch�ؾ��Ѵ�.
	while( pODBC_DBResultSet->next() == 1){
		::ZeroMemory( id, BUFFER_SIZE );
		::ZeroMemory( passwd, BUFFER_SIZE );
		
		pODBC_DBResultSet->getString( id, BUFFER_SIZE );
		pODBC_DBResultSet->getString( passwd, BUFFER_SIZE );

		printf( "ID : %s / PASSWORD : %s\n", id, passwd );
	}	//end while

	return NULL;
}

/* QueryAdder */
class QueryAdder{
public:
	QueryAdder( DBQueryQ *pDBQueryQ ){
		this->pDBQueryQ = pDBQueryQ;
	}

	inline void add( char *query ){
		this->pDBQueryQ->add_query( query, strlen( query ) );
	}
private:
	DBQueryQ *pDBQueryQ;
};

/* executeQuery */
int executeQuery( DBWork *pDBWork, char *query ){
	printf("execute query... : %s\n", query );

	/* 1. DB ���� �������� */
	// DBSessionProxy�� ���� DBService���� DBSession�� �����´�.
	DBSessionProxy DBProxy( pDBWork->getDBName() );
	ODBC_DBSession *pDBSession = (ODBC_DBSession *)DBProxy.get();

	/* 2. ���� ���� */
	ODBC_DBStatement *pDBStatement = (ODBC_DBStatement *) pDBSession->get_statement();
	
	// !! prepare �޼ҵ带 ���� ȣ���ؼ�, SQL �ڵ��� �Ҵ��ؾ� ������ ������ �� �ִ�. !!
	// result_type_�� ���� ������ ����
	pDBStatement->prepare( DBResultSet::DEF_RESULT_TYPE_ROW );

	pDBStatement->set_sql( query );
	ODBC_DBResultSet *pDBResultSet = (ODBC_DBResultSet *) pDBStatement->executeQuery();
	
	/* 3. ���� üũ */
	ODBC_DBError *pError = (ODBC_DBError *)pDBSession->get_error();

	if( pError->getErrorCode() != 0 ){
		printf("DBError(%d) : %s\n", pError->getErrorCode(), pError->getMsg(0) );
		return pError->getErrorCode();
	}

	/* 4. result set ó�� */
	pDBWork->resultSetHandler( pDBResultSet );

	/* 5. Session ���� */
	DBProxy.release();
	return 0;
};

/* QueryListener */
class QueryListener : public WorkListener{
public:
    int workPerformed( Work* work, void* arg );
	int specificPerformed( Work* work, void* arg );
	QueryListener() : WorkListener(0){}
};

int QueryListener::workPerformed( Work* work, void* arg ){
	return executeQuery( (DBWork *) work, (char *) arg );
}

int QueryListener::specificPerformed( Work* work, void* arg ){
	return 0;
}

int main(){
	// 1. DBQueryQ�� ���� DB ���̺귯�� ��� ����
	// 1-1. DBSerivce�� ���� ���Ϸ� �ʱ�ȭ�Ѵ�.
	if( DBService::Instance()->initialize("environment.xml") ){
		printf("Error : DBService::initialize() failed\n");
		return -1;
	}
	
	// 1-2. ������ ó���� WorkListener�� �����.
	QueryListener qListener;

	// 1-3. WorkListener::workPerformed()���� ���� work�� �����.
	ODBC_DBWork dbWork( "test" );
	
	// 1-4. ���� ť �����带 �����Ͽ� WorkListener���� ������ �޾� ó���Ѵ�.
	// ( executeQuery() �ּ� ���� )
	DBQueryQ DBQueryQ_Thread;
	if( false == DBQueryQ_Thread.initialize( &qListener, &dbWork, false ) ){
		printf("Error : DBQueryQ_Thread::initialize() failed\n");
		return -1;
	}

	QueryAdder qAdder( &DBQueryQ_Thread );

	Sleep( 200 );

	printf("\n\nDBQueryQ\n\n");

	// 1-4.1. DBQueryQ::addQuery()�� ���� ������ �����Ѵ�. ������ WorkListener::workPerformed()�� ���޵ȴ�.
	qAdder.add( "BEGIN TRAN" );
	qAdder.add( "create table userInfo ( id nchar(30), pw nchar(30) )" );
	qAdder.add( "insert userInfo values( 'USER1', '1234')"  );
	qAdder.add( "insert userInfo values( 'gildong', 'abcd')" );
	qAdder.add( "select * from userInfo" );
	qAdder.add( "ROLLBACK" );

	/*************************************************************/
	Sleep(500);
	/*************************************************************/
	
	// 2. DBQueryQ�� ������� �ʴ� DB ���̺귯�� ��� ����
	// 2-1. DBSerivce�� ���� ���Ϸ� �ʱ�ȭ�Ѵ�. (����)
	// 2-2. ���� ���������� proxy�� ���� DB������ �޾� ������ �����Ѵ�.
	// ( executeQuery() �ּ� ���� )

	printf("\n\nDBProxy\n\n");
	
	executeQuery( &dbWork, "BEGIN TRAN" );
	executeQuery( &dbWork, "create table userInfo ( id nchar(30), pw nchar(30) )" );
	executeQuery( &dbWork, "insert userInfo values( 'mgame', '0987')"  );
	executeQuery( &dbWork, "insert userInfo values( 'cultwo', 'zxcv')");
	executeQuery( &dbWork, "insert userInfo values( 'godori', '7777')");
	executeQuery( &dbWork, "select * from userInfo" );
	executeQuery( &dbWork, "delete from userInfo where id = 'godori'" );
	executeQuery( &dbWork, "select * from userInfo" );
	executeQuery( &dbWork, "ROLLBACK" );
	
	// ���� ����
	printf("\n");
	executeQuery( &dbWork, "example : invalid sql!" );
	
	while(1)
		Sleep(100);

	return 0;
}

