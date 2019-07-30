MODULE TextsCP;


IMPORT RTS,Console,texts;

TYPE Reader* =RECORD eot*:BOOLEAN;eol*:BOOLEAN;
				filename*:RTS.NativeString; jReader:texts.Texts;
			   END;
	 Writer*= RECORD eot*:BOOLEAN;filename*:ARRAY 80 OF CHAR; 
	 
	 END; 
CONST ok=TRUE; (*test only for output syntaxdriven desktop 
	false for syndred on server*)

PROCEDURE  WriteStringWriter*(W:Writer; Str:ARRAY OF CHAR ); 

BEGIN
	Console.WriteString(Str);
END WriteStringWriter;

PROCEDURE Write*(ch:CHAR);
BEGIN
	IF ok THEN Console.Write(ch);END;
END Write;

PROCEDURE  WriteString*(Str:ARRAY OF CHAR ); 

BEGIN
	IF ok THEN Console.WriteString(Str);END;
END WriteString;

PROCEDURE  WriteStringLn*(Str:ARRAY OF CHAR ); 
 
BEGIN
	IF ok THEN
		Console.WriteString(Str);
		Console.WriteLn();
	END;	
END WriteStringLn;

PROCEDURE WriteInt*(pos:INTEGER; len:INTEGER); 
BEGIN
	IF ok THEN Console.WriteInt(pos,len);
	END;
END WriteInt;

PROCEDURE WriteIntWriter*(W:Writer; pos:INTEGER; len:INTEGER); 
BEGIN
	Console.WriteInt(pos,len);
	
END WriteIntWriter;

PROCEDURE WriteLn*(); 
BEGIN
	IF ok THEN 
		Console.WriteLn();
	END;
END WriteLn;

PROCEDURE OpenWriter*(W:Writer);
BEGIN
	W.eot:=FALSE;
	WriteString("OpenWriter");WriteLn();
	
END OpenWriter;


PROCEDURE Pos*(R:Reader):INTEGER;
BEGIN
	RETURN 0;
END Pos;

PROCEDURE OpenReader*(VAR R:Reader);
BEGIN
	WriteString("OpenReader "+R.filename);WriteLn();
	NEW(R.jReader);	
	R.jReader.open(MKSTR(R.filename));R.eot:=FALSE;
END OpenReader;



PROCEDURE Read*(VAR R:Reader;VAR ch:CHAR);
BEGIN	
		(* eof error *)
		IF R.jReader.eot THEN 
			WriteString("Texts.Read EOF error for file: "+R.filename);
			RTS.Throw("Texts.Read EOF error");
		ELSE
			ch:=R.jReader.readCharFromFile();
			Write(ch);
			IF (ch=0DX) OR (ch=0AX) THEN R.eol:=TRUE;ch:=' ';
			ELSE R.eol:=FALSE;
			END;
			IF R.jReader.eot THEN 
				R.eot:=TRUE; 	
				WriteStringLn("Texts.Read eot set true");
			END;
		END;
END Read;

PROCEDURE Length*(text:ARRAY OF CHAR):INTEGER;

VAR len:INTEGER;

BEGIN
	len:=0;
	WHILE text[len] # 0X DO
		INC(len);
	END;
	RETURN len;

END Length;

BEGIN (*Auto-generated*)
	
END TextsCP.