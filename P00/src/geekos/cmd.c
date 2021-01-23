#include <geekos/bootinfo.h>
#include <geekos/string.h>
#include <geekos/screen.h>
#include <geekos/mem.h>
#include <geekos/crc32.h>
#include <geekos/tss.h>
#include <geekos/int.h>
#include <geekos/kthread.h>
#include <geekos/trap.h>
#include <geekos/timer.h>
#include <geekos/keyboard.h>

static void Start_Project(ulong_t arg) {
	static int x,y,idx,saveX;	// 변수 선언 x,y 커서의 좌표  saveX X좌표의 위치를 기억
	static int saveY[24]={1,};	// SaveY배열 각각 줄의 끝 위치를 기억하고 있다.
	bool last_line= false;		// 현재 위치가 라인의 끝위치를 검사해주는 변수
	bool shift_flag=false;		// 블럭 잡은 곳을 복사 하는 경우를 나누는 flag
	char board[24][80]={0,};	// 24 X 80 배열 선언 모든 화면을 기억하기 위한 배열
    char temp[50];				// 블럭이 잡힌 잘라내기와 복사한 문자를 저장 	
	int z=0;					// for 문을 위한 변수.
	bool bcountl = true;	
	bool bcountr = true;		// 블럭의 활성상태를 결정하는 변수
	int count_flag = 0;			// 블럭의 크기 변수
	int count_flag2 = 0;		// 블럭의 크기를 정하는 변수를 저장
	int count_dump=0;			// 복사를 위한 dump 변수
	Keycode key;				// 구조체 선언
	
	Set_Current_Attr(ATTRIB(BLACK, BLUE|BRIGHT));//글자 배경과 색을 변환하는 함수
	Print("		Samsung Software Membership the 21th applicant 'JANG JUN HO' \n");
	Set_Current_Attr(ATTRIB(BLACK, GRAY));
	
	while(1) {
		key = Wait_For_Key();			  // Keyboard 입력을 대기
    	if((key & KEY_RELEASE_FLAG) == 0) // key 입력을 받은 경우 수행
		{	
			switch(key) //switch 문
			{
				case 0x0d : // Pushed Enter 
					Get_Cursor(&x,&y);
					
					// 블럭이 잡혔을 경우 해제. 현위치에서 좌로 공백 입력
					if (count_flag > 0) {
						if(y - count_flag2 > 0)					
							Put_Cursor(x,y-count_flag2);
						else
							Put_Cursor(x,y);
						for(z=0 ; z <count_flag2 ; z++)
						{
							Get_Cursor(&x,&y);
							Print("%c",board[x][y]);
							y++;
						}
						count_flag = 0; count_flag2 = 0;
					}
					Get_Cursor(&x,&y);
					saveY[idx] = y; // 끝 위치를 저장
					idx++;			// idx 는 현 커서에 x(row) 값 ++
					Print("\n");
					Get_Cursor(&x,&y);
					bcountl = true; bcountr = true; count_flag2 = 0; // 블럭 flag 들을 초기화
					break;
			   case 0x08 : // pushed backspace 
					Get_Cursor(&x,&y);
					// y==0 커서에 위치가 0,0 일 경우 위 맨 뒷줄로 올라가 삭제한다.		
					if(y==0) {
						idx--;
						Put_Cursor(x-1,79);
						Print(" ");
						Put_Cursor(x-1,saveY[idx]); // 커서를 윗줄의 마지막 줄로 이동
						if(last_line)
						{									
							Print(" ");
							Put_Cursor(x-1,saveY[idx]);			 
							last_line=false;
						}
					}
					// 일반적인 삭제(커서를 좌로 옮긴 후 공백 입력)
					else {
						Put_Cursor(x,y-1);
						Print(" ");
						Put_Cursor(x,y-1);
						saveY[idx] -= 1;
					}
					bcountl = true; bcountr = true; count_flag2 = 0;
				break;
				// Ctrl + D 를 누른 경우 종료
			case 0x4064 : // CTRL+D 
				Print("\nbye"); // 출력
				Exit(0);		// 커널쓰레드 0번(Project0) 종료
   				break;
			case 0x4063 : // CTRL+C
					if(bcountr==true) {// 블럭이 잡힌 경우(좌에서 우로) 수행: 블럭 잡힌 곳에 문자를 temp 배열에 저장한다.
						z=0;
						if(shift_flag) {// 블럭이 잡힌 상태
							for( ; z < count_flag ; y++,z++) {
								temp[z]=board[x][y-count_flag+1];
							}
							shift_flag=false;
						}
		            } else { // 블럭이 잡힌 경우(우에서 좌로) 수행: 블럭 잡힌 곳에 문자를 temp 배열에 저장한다.					
						z=0;
						y--;
				        if(shift_flag) { // 블럭이 잡힌 상태
							for( ;  z < count_flag ; y++,z++) {
								temp[z]=board[x][y];
							}
							shift_flag=false;
						}
					}
					count_dump = count_flag; // 복사 크기를 고정하기 위해 count_dump 변수에 블럭잡힌 크기를 저장.
					count_flag = 0; bcountl = true; bcountr = true; // 블럭 상태 초기화
				break;
			case 0x4078 : // CTRL+X 
				if(bcountr==true) { // 블럭이 잡힌 경우(좌에서 우로) 수행: 잡힌 곳에 문자를 " " 으로 대체하고 temp 배열에 저장한다.	
					z=0;
					if(shift_flag) { // 블럭이 잡힌 상태
						for( ;  z < count_flag ; y++,z++) {
							temp[z]=board[x][y-count_flag+1];
							Put_Cursor(x,y-count_flag+1);
							Print(" ");
							board[x][y-count_flag+1]=NULL;
						}
						shift_flag=false;
					}
				} else { // 블럭이 잡힌 경우(우에서 좌로) 수행: 잡힌 곳에 문자를 " " 으로 대체하고 temp 배열에 저장한다.				
					z=0;
					y--;
					if(shift_flag) { //블럭이 잡힌 상태
						for( ; z < count_flag ; y++,z++) {
							temp[z]=board[x][y];
							Put_Cursor(x,y);
							Print(" ");
							board[x][y]=NULL;
						}
						shift_flag=false;
					}
				}
				count_dump = count_flag;
				count_flag = 0;
   				bcountl = true; bcountr = true; 
				break;
			case 0x4076 : // CTRL+V temp 배열에 저장되어 있는 것을 현위치에서 count_dump 크기만큼 뿌려준다.
				Get_Cursor(&x,&y);
				saveX = x;
			// 블럭이 잡힌 상태(우에서 좌로)를 해제
			if(y-count_flag2 > 0) {
				Put_Cursor(x,y-count_dump);
				for(z=0 ; z < count_dump ; z++) {
					Get_Cursor(&x,&y);
					Print("%c",board[x][y++]);
					Put_Cursor(x,y++);
				}			
			}
			// temp배열 내용을 출력
			Get_Cursor(&x,&y);
			for(z=0 ; z <count_dump ; z++) {
				board[x][y] = temp[z];
				y++;
				Print("%c",temp[z]);
				board[x][y++] = temp[z];
				Get_Cursor(&x,&y);	
			}
			if(saveY[idx] < y) // 붙여넣기를 할 경우 끝 위치가 더 커질 경우 현재 끝위치(col)를 saveY배열에 저장
				saveY[idx] = y;
			if(x == saveX+1) {	//end set
				saveY[idx++] = 79;
				saveY[idx] = y;
			}
		    bcountl = true; bcountr = true;    	//블럭 상태 초기화
   			break;	
			case (0x0200 | 0x0100 | (4+KEYPAD_START)) :  // LEFT Key 
			// 블럭을 잡고, 방향키를 누를 경우 블럭을 없애는 작업(좌우를 흑백키로 재 출력) 
				if(count_flag2 > 0) { // 블럭이 활성상태에서만 실행
					Set_Current_Attr(ATTRIB(BLACK, GRAY));
					Get_Cursor(&x,&y);
					Put_Cursor(x,y-count_flag2);
					if(y-count_flag2 >0) { // 현 커서에 col 값보다 블럭크기가 더 작을경우 실행(블럭을 해제해도 커서위치를 고정) 
						for(z=0 ; z <count_flag2 ; z++) { // 좌측 재입력
							Get_Cursor(&x,&y);
							Print("%c",board[x][y]);
							y++;
						}
					}						
					for(z=0 ; z <count_flag2 ; z++) { //우측 재입력
						Get_Cursor(&x,&y);
						Print("%c",board[x][y]);
						y++;
					}
					Put_Cursor(x,y-count_flag2+1);
					count_flag = 0;
					count_flag2 = 0;
				} 
				// 커서위치 이동//////////////////////////////////////
				Get_Cursor(&x,&y);
				Put_Cursor(x,y-1);
				bcountl = true; bcountr = true;
				break;
			case (0x0200 | 0x0100 |  (6+KEYPAD_START)) : //RIGHT
				// 블럭을 잡고, 방향키를 누를 경우 블럭을 없애는 작업(좌우를 흑백키로 재 출력)
				if(count_flag2 > 0) { //블럭이 활성상태에서만 실행
					Set_Current_Attr(ATTRIB(BLACK, GRAY));
					Get_Cursor(&x,&y);
					Put_Cursor(x,y-count_flag2);
				if(y-count_flag2 >0) { // 현 커서에 col 값보다 블럭크기가 
									   // 더 작을경우 실행(블럭을 해제해도 커서위치를 고정) 	
					for(z=0 ; z <count_flag2 ; z++) // 좌측 재입력
					{
						Get_Cursor(&x,&y);
						Print("%c",board[x][y]);
						y++;
					}
				}
					for(z=0 ; z <count_flag2 ; z++) //우측 재입력
					{
						Get_Cursor(&x,&y);
						Print("%c",board[x][y]);
						y++;
					}
					Put_Cursor(x,y-count_flag2-1);							
					count_flag = 0;
					count_flag2 = 0;
				  } 
					// 커서위치 이동//////////////////////////////////////
					Get_Cursor(&x,&y);
					Put_Cursor(x,y+1);
					bcountl = true; bcountr = true;
					break;

			case (0x0200 | 0x0100 | (1+KEYPAD_START)) : // UP
			// 블럭을 잡고, 방향키를 누를 경우 블럭을 없애는 작업(좌우를 흑백키로 재 출력)
				if(count_flag2 > 0)	 //블럭이 활성상태에서만 실행
				{
					Set_Current_Attr(ATTRIB(BLACK, GRAY));
					Get_Cursor(&x,&y);
					Put_Cursor(x,y-count_flag2);
				if(y-count_flag2 >0){ //현커서에 col 값보다 블럭크기가 
									//더 작을경우 실행(블럭을 해제해도 커서위치를 고정) 
					for(z=0 ; z <count_flag2 ; z++) //좌측 재입력
					{
						Get_Cursor(&x,&y);
						Print("%c",board[x][y]);
						y++;
					}
				}	
					for(z=0 ; z <count_flag2 ; z++) //우측 재입력
					{
						Get_Cursor(&x,&y);
						Print("%c",board[x][y]);
						y++;
					}
					Put_Cursor(x,y-count_flag2);							
					count_flag = 0;
					count_flag2 = 0;
				  } 
					// 커서위치 이동
					Get_Cursor(&x,&y);
					Put_Cursor(x-1,y);
					idx--;
					bcountl = true; bcountr = true;
					break;

			case (0x0200 | 0x0100 |  (9+KEYPAD_START)) : //DOWN
			// 블럭을 잡고, 방향키를 누를 경우 블럭을 없애는 작업(좌우를 흑백키로 재 출력) //////////////////
				if(count_flag2 > 0){ //블럭이 활성상태에서만 실행
					Set_Current_Attr(ATTRIB(BLACK, GRAY));
					Get_Cursor(&x,&y);
					Put_Cursor(x,y-count_flag2);
				if(y-count_flag2 >0){ //현커서에 col 값보다 블럭크기가 
										//더 작을경우 실행(블럭을 해제해도 커서위치를 고정) 
					for(z=0 ; z <count_flag2 ; z++) //좌측 재입력
					{
						Get_Cursor(&x,&y);
						Print("%c",board[x][y]);
						y++;
					}
				}
										
					for(z=0 ; z <count_flag2 ; z++) //우측 재입력
					{
						Get_Cursor(&x,&y);
						Print("%c",board[x][y]);
						y++;
					}
					Put_Cursor(x,y-count_flag2);							
					count_flag = 0;
					count_flag2 = 0;
				  } 
					// 커서위치 이동
					Get_Cursor(&x,&y);
					Put_Cursor(x+1,y);
					idx++;
					bcountl = true; bcountr = true; count_flag2 = 0; //블럭 상태 초기화
					break;
			case (0x0200 | 0x0100 |  (0+KEYPAD_START)) :	//home
				// 블럭을 잡고, 방향키를 누를 경우 블럭을 없애는 작업(좌우를 흑백키로 재 출력) //////////////////
				if(count_flag2 > 0){ //블럭이 활성상태에서만 실행
					Set_Current_Attr(ATTRIB(BLACK, GRAY));
					Get_Cursor(&x,&y);
					Put_Cursor(x,y-count_flag2);
				if(y-count_flag2 >0){ //현커서에 col 값보다 블럭크기가 
										//더 작을경우 실행(블럭을 해제해도 커서위치를 고정)
					for(z=0 ; z <count_flag2 ; z++)//좌측 재입력
					{
						Get_Cursor(&x,&y);
						Print("%c",board[x][y]);
						y++;
					}
				}
										
					for(z=0 ; z <count_flag2 ; z++)//우측 재입력
					{
						Get_Cursor(&x,&y);
						Print("%c",board[x][y]);
						y++;
					}
					Put_Cursor(x,y-count_flag2);							
					count_flag = 0;
					count_flag2 = 0;
				  } 					
				// 커서위치 이동
				Get_Cursor(&x,&y);
					count_flag2 = 0;
					Put_Cursor(x,0); //커서를 현 row에서 가장 왼쪽으로 이동.
					bcountl = true; 	bcountr = true; count_flag2 = 0; //블럭
					break;
			case (0x0200 | 0x0100 |  (8+KEYPAD_START)) :	// end
			// 블럭을 잡고, 방향키를 누를 경우 블럭을 없애는 작업(좌우를 흑백키로 재 출력)
				if(count_flag2 > 0){ //블럭이 활성상태에서만 실행
					Set_Current_Attr(ATTRIB(BLACK, GRAY));
					Get_Cursor(&x,&y);
					Put_Cursor(x,y-count_flag2);
				if(y-count_flag2 >0){ //현커서에 col 값보다 블럭크기가 
										//더 작을경우 실행(블럭을 해제해도 커서위치를 고정)
					for(z=0 ; z <count_flag2 ; z++)//좌측 재입력
					{
						Get_Cursor(&x,&y);
						Print("%c",board[x][y]);
						y++;
					}
				}
										
					for(z=0 ; z <count_flag2 ; z++)//우측 재입력
					{
						Get_Cursor(&x,&y);
						Print("%c",board[x][y]);
						y++;
					}
					Put_Cursor(x,y-count_flag2);							
					count_flag = 0;
					count_flag2 = 0;
				  } 
					// 커서위치 이동
					Get_Cursor(&x,&y);
					Put_Cursor(x,saveY[idx]);  //커서를 saveY 배열에 저장되어 있는 위치(가장 끝 문자)로 이동(idx는 row를 가리킴)
					bcountl = true; 	bcountr = true; count_flag2 = 0; //초기화
					break;
                        case (0x0200 | 0x0100 |  (12+KEYPAD_START)) :	//delete
				// 블럭을 잡고, 방향키를 누를 경우 블럭을 없애는 작업(좌우를 흑백키로 재 출력) 
				if(count_flag2 > 0){ // 블럭을 잡고, 방향키를 누를 경우 블럭을 없애는 작업.
					Set_Current_Attr(ATTRIB(BLACK, GRAY));
					Get_Cursor(&x,&y);
					Put_Cursor(x,y-count_flag2);
				if(y-count_flag2 >0){ //현커서에 col 값보다 블럭크기가
									//더 작을경우 실행(블럭을 해제해도 커서위치를 고정)
					for(z=0 ; z <count_flag2 ; z++)//좌측 재입력
					{
						Get_Cursor(&x,&y);
						Print("%c",board[x][y]);
						y++;
					}
				}
										
					for(z=0 ; z <count_flag2 ; z++)//우측 재입력
					{
						Get_Cursor(&x,&y);
						Print("%c",board[x][y]);
						y++;
					}
					Put_Cursor(x,y-count_flag2);							
					count_flag = 0;
					count_flag2 = 0;
				  } 
					// 공백을 입력해 오른쪽 문자를 지운다.
					Get_Cursor(&x,&y);					
					Print(" ");
					board[x][y] = " ";
					Put_Cursor(x,y+1); bcountl = true; 	bcountr = true; count_flag2 = 0;// 블럭 상태 초기화
					break;
			case (0x1000 + (0x0200 | 0x0100 |  (1+KEYPAD_START))) :
					break;
			case (0x1000 + (0x0200 | 0x0100 |  (9+KEYPAD_START))) :
					break;
			case (0x1000 + (0x0200 | 0x0100 |  (6+KEYPAD_START))) :// shift + ->
				if(shift_flag==false)  //쉬프트 flag 변수를 두어서 블럭을 잡는 상태인지를 표시 (Ctrl + C 를 위함) 
				    shift_flag=true;

				if (bcountr){ //블럭을 잡는 부분.
				Get_Cursor(&x,&y);
						
				Set_Current_Attr(ATTRIB(BLACK, GREEN|BRIGHT)); // 
				Put_Char(board[x][y]);
				Set_Current_Attr(ATTRIB(BLACK, GRAY));
				
				bcountl = false;
				++count_flag; //블럭 크기를 계산하기 위해 잡을때마다 ++
				count_flag2 = count_flag; // 블럭 크기를 두 군데서 저장(2는 블럭 해제할시 사용된다.)
				if(count_flag == 0){
 					bcountl = true; 	bcountr = true;
					}				
				}
				else{  //블럭을 제거 하는 부분.
				
				Get_Cursor(&x,&y);
				Set_Current_Attr(ATTRIB(BLACK, GRAY));
				Put_Char(board[x][y]);
				Set_Current_Attr(ATTRIB(BLACK, GREEN|BRIGHT));
				
				bcountl = true;
				--count_flag; //블럭 크기를 계산하기 위해 해제 할때마다 --
				count_flag2 = count_flag;  // 블럭 크기를 두 군데서 저장(각각 다른 곳에 쓰임)
				if(count_flag == 0){
 					bcountl = true;
					bcountr = true;

					}			
				}

				break;
			case (0x1000 + (0x0200 | 0x0100 |  (4+KEYPAD_START))) : //shift + <-

				if(shift_flag==false)   //쉬프트 flag 변수를 두어서 블럭을 잡는 상태인지를 표시 (Ctrl + C 를 위함) 
				{
				  
				    shift_flag=true;
				}
				if (bcountl){ // 블럭을 잡는 부분

				Get_Cursor(&x,&y);
						
				Set_Current_Attr(ATTRIB(BLACK, GREEN|BRIGHT));
				Put_Cursor(x,y-1);				
				Put_Char(board[x][y-1]);
				Put_Cursor(x,y-1);
				Set_Current_Attr(ATTRIB(BLACK, GRAY));
				bcountr = false;
				++count_flag; //블럭 크기를 계산하기 위해 잡을때마다 ++
				count_flag2 = count_flag;  // 블럭 크기를 두 군데서 저장(2는 블럭 해제할시 사용된다.)
				if(count_flag == 0){
 					bcountl = true;
					bcountr = true;
	
					}				
				}
			  	else{ //블럭을 제거 하는 부분.
				
				Get_Cursor(&x,&y);
				Set_Current_Attr(ATTRIB(BLACK, GRAY));
				Put_Cursor(x,y-1);				
				Put_Char(board[x][y-1]);
				Put_Cursor(x,y-1);
				Set_Current_Attr(ATTRIB(BLACK, GREEN|BRIGHT));
				bcountr = true;
				--count_flag;  //블럭 크기를 계산하기 위해 해제 할때마다 --
				count_flag2 = count_flag;  // 블럭 크기를 두 군데서 저장(2는 블럭 해제할시 사용된다.)
				if(count_flag == 0){
 					bcountl = true;
					bcountr = true;
					}
				}
				break;
									
			default : // pushed usual key

			// 블럭을 잡고, 방향키를 누를 경우 블럭을 없애는 작업(좌우를 흑백키로 재 출력) 
				if(count_flag2 > 0){ //블럭이 활성상태에서만 실행
					Set_Current_Attr(ATTRIB(BLACK, GRAY));
					Get_Cursor(&x,&y);
					Put_Cursor(x,y-count_flag2);
				if(y-count_flag2 >0){
					for(z=0 ; z <count_flag2 ; z++)
					{
						Get_Cursor(&x,&y);
						Print("%c",board[x][y]);
						y++;
					}
				}
										
					for(z=0 ; z <count_flag2 ; z++)
					{
						Get_Cursor(&x,&y);
						Print("%c",board[x][y]);
						y++;
					}
					Put_Cursor(x,y-count_flag2);							
					count_flag = 0;
					count_flag2 = 0;
				  } 

				Get_Cursor(&x,&y);
				if(y==79) { //끌(col) 인 경우
					last_line=true;
					saveY[idx] = 79;
					idx++;
					board[x][y]=key;
					Print("%c",board[x][y]);
					}
				else{
					board[x][y]=key;					
					Print("%c",board[x][y]);
					if(saveY[idx] < y)
						saveY[idx] = y;
				}
				
				bcountl = true; bcountr = true;
				break;
			}
			if(x==24){ //끝(row) 인 경우
				Clear_Screen(); //화면을 Clear한다.
				Put_Cursor(0,0);
				bcountl = true; bcountr = true;
			}
		}
   }
}
				

