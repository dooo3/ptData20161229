/* 예제 실행 조건 :
	1. Base Lib
	2. DB Lib (../lib 디렉토리의 tinyxml.lib, mysql/ibmysql.lib, mysql/libmysql.dll 추가 필요)
	3. SQL 서버 설정 및 ODBC 설정
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

	// ResultSet에서 처음에 값을 읽기 전에 next()를 실행해서 fetch해야한다.
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

	/* 1. DB 세션 가져오기 */
	// DBSessionProxy를 통해 DBService에서 DBSession을 가져온다.
	DBSessionProxy DBProxy( pDBWork->getDBName() );
	ODBC_DBSession *pDBSession = (ODBC_DBSession *)DBProxy.get();

	/* 2. 쿼리 실행 */
	ODBC_DBStatement *pDBStatement = (ODBC_DBStatement *) pDBSession->get_statement();
	
	// !! prepare 메소드를 먼저 호출해서, SQL 핸들을 할당해야 쿼리를 실행할 수 있다. !!
	// result_type_은 현재 쓰이지 않음
	pDBStatement->prepare( DBResultSet::DEF_RESULT_TYPE_ROW );

	pDBStatement->set_sql( query );
	ODBC_DBResultSet *pDBResultSet = (ODBC_DBResultSet *) pDBStatement->executeQuery();
	
	/* 3. 에러 체크 */
	ODBC_DBError *pError = (ODBC_DBError *)pDBSession->get_error();

	if( pError->getErrorCode() != 0 ){
		printf("DBError(%d) : %s\n", pError->getErrorCode(), pError->getMsg(0) );
		return pError->getErrorCode();
	}

	/* 4. result set 처리 */
	pDBWork->resultSetHandler( pDBResultSet );

	/* 5. Session 해제 */
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
	// 1. DBQueryQ를 통한 DB 라이브러리 사용 예제
	// 1-1. DBSerivce를 세팅 파일로 초기화한다.
	if( DBService::Instance()->initialize("environment.xml") ){
		printf("Error : DBService::initialize() failed\n");
		return -1;
	}
	
	// 1-2. 쿼리를 처리할 WorkListener를 만든다.
	QueryListener qListener;

	// 1-3. WorkListener::workPerformed()에서 받을 work을 만든다.
	ODBC_DBWork dbWork( "test" );
	
	// 1-4. 쿼리 큐 스레드를 실행하여 WorkListener에서 쿼리를 받아 처리한다.
	// ( executeQuery() 주석 참조 )
	DBQueryQ DBQueryQ_Thread;
	if( false == DBQueryQ_Thread.initialize( &qListener, &dbWork, false ) ){
		printf("Error : DBQueryQ_Thread::initialize() failed\n");
		return -1;
	}

	QueryAdder qAdder( &DBQueryQ_Thread );

	Sleep( 200 );

	printf("\n\nDBQueryQ\n\n");

	// 1-4.1. DBQueryQ::addQuery()를 통해 쿼리를 삽입한다. 쿼리는 WorkListener::workPerformed()로 전달된다.
	qAdder.add( "BEGIN TRAN" );
	qAdder.add( "create table userInfo ( id nchar(30), pw nchar(30) )" );
	qAdder.add( "insert userInfo values( 'USER1', '1234')"  );
	qAdder.add( "insert userInfo values( 'gildong', 'abcd')" );
	qAdder.add( "select * from userInfo" );
	qAdder.add( "ROLLBACK" );

	/*************************************************************/
	Sleep(500);
	/*************************************************************/
	
	// 2. DBQueryQ를 사용하지 않는 DB 라이브러리 사용 예제
	// 2-1. DBSerivce를 세팅 파일로 초기화한다. (생략)
	// 2-2. 위와 마찬가지로 proxy를 통해 DB세션을 받아 쿼리를 실행한다.
	// ( executeQuery() 주석 참조 )

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
	
	// 에러 예제
	printf("\n");
	executeQuery( &dbWork, "example : invalid sql!" );
	
	while(1)
		Sleep(100);

	return 0;
}

