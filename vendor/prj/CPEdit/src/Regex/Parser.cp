MODULE RegexParser;

IMPORT texts,TextsCP, RTS,FontsFont;



TYPE 
	Regex*=POINTER TO RegexType;
	RegexType*=EXTENSIBLE RECORD
		lastPiece*:Piece;
		last:Branch;
		branch-:Branch;
		Font*: FontsFont.FontDesc;
	END;

Branch*=POINTER TO RECORD
	alt-:Branch;
	piece-:Piece;
END;

Piece*=POINTER TO RECORD
	suc*:Piece;
	atom-:Atom;
	min-,max-:Quantifier;
	(*matchProc-:MatchProc;*)
	id-:INTEGER;
END;

Atom*=POINTER TO RECORD
	range-:Range;
	regex-:Regex;
END;

Range*=POINTER TO RECORD
	pos-,sub-:BOOLEAN;
	min-,max-:CHAR;
	next-:Range
END;

Quantifier*=POINTER TO RECORD
	val-:INTEGER
END; 


CONST char=0;asterisk=1;plus=2;qum=3;bar=4;osquareBr=5;csquareBr=6;neg=7 (*^*);escape=8;lKlammer=9;rKlammer=10;glKlammer=11;grKlammer=12;strich=13;punkt=14; other=15;

VAR 	ok:BOOLEAN;
		regex:Regex;
		errString:ARRAY 128 OF CHAR;
		regString:POINTER TO ARRAY OF CHAR;
		sym:INTEGER;
		i:INTEGER;
		ch:CHAR;
		iden:CHAR;
		digits:ARRAY 32 OF CHAR;
		er:INTEGER;
		error:BOOLEAN;
		globalLast*:Piece;
	
(*--------------------------------Fehler-Meldungen------------------------------------*)
PROCEDURE Error(id:INTEGER);
VAR e:RTS.NativeException;
BEGIN
	TextsCP.WriteLn(); TextsCP.WriteString(regString); TextsCP.WriteLn();
	er:=0;
	FOR er:=0 TO i-1 DO errString[er]:=" " END;
	errString[i]:="^";
	errString[i+1]:=0X;
	TextsCP.WriteString(errString);
	TextsCP.WriteLn();
	CASE id OF 
		1: TextsCP.WriteString("Fehler. Erwartet: char oder \ oder . oder [ oder ( "); TextsCP.WriteLn();
    	error:=TRUE;
		|2: TextsCP.WriteString("Fehler. Keine gueltige Escape-Sequenz"); TextsCP.WriteLn();
		error:=TRUE;
		|3: TextsCP.WriteString("Fehler. Erwartet )");TextsCP.WriteLn();
			error:=TRUE;
		|4: TextsCP.WriteString("Fehler. Erwartet ]");TextsCP.WriteLn();
			error:=TRUE;
		|5: TextsCP.WriteString("Fehler. Keine gueltige CharGroup");TextsCP.WriteLn();
		|6:TextsCP.WriteString("Default. Noch keine Spezifikation");TextsCP.WriteLn();
		error:=TRUE;
	
	END;
	IF error THEN 
		NEW(e);
		THROW(e);
	END;
END Error;
(*ELSE TextsCP.WriteLn(); TextsCP.WriteString(regString); TextsCP.WriteLn();
			
			
			(*Create Error Mark*)
			(* an error-Proc Fehlercode أ¼bergeben; anhand dessen die erwarteten
				syms evaluiert werden kأ¶nnen *)
			er:=0;
			FOR er:=0 TO i-2 DO errString[er]:=" " END;
			errString[i-2]:="^";
			TextsCP.WriteString(errString);
			TextsCP.WriteLn();
			TextsCP.WriteString("Fehler. Erwartet: char oder [ oder (");
			TextsCP.WriteLn();
			Interface_Halt.halt.HaltPar(127)	 *)

(*-------------------------------Scanner-Procedures---------------------------------*)
PROCEDURE Read();
BEGIN
	ch:=regString^[i];INC(i);
	
END Read;

PROCEDURE GetSym();
BEGIN
	CASE ch OF	   	
	CHR(1)..CHR(39),
	CHR(44),
	CHR(47)..CHR(62),
	CHR(64)..CHR(90), 
	CHR(94  )..CHR(122),
	CHR(126)..CHR(255),
	(*greek*)
	CHR(880)..CHR(1023):sym:=char;iden:=ch; Read(); (*alles auكer Metachar*)
	| "*":sym:=asterisk; Read();
	| "+":sym:=plus; Read();
	| "?":sym:=qum; Read();
	| "|":sym:=bar; Read();
	| "[":sym:=osquareBr; Read();
	| "]":sym:=csquareBr; Read();
	| "\":sym:=escape; Read();
	| "(":sym:=lKlammer; Read();
	| ")":sym:=rKlammer; Read();
	| "{":sym:=glKlammer; Read();
	| "}":sym:=grKlammer; Read();
	| "-":sym:=strich;Read();  (*auch nicht Metachar; Algorithmus أ¤ndern *)
	| ".":sym:=punkt;Read();
	ELSE sym:=other
	END;
	TextsCP.WriteString(" GetSym sym: ");
	TextsCP.WriteInt(sym,2);
	TextsCP.WriteLn;
	
	
END GetSym;

(*-------------------------------Parsing-Procedures----------------------------------*)
PROCEDURE MultiCharEsc (VAR range:Range);
VAR new,last:Range;
BEGIN
	CASE sym OF 		    	
	punkt:(*[^\n\r]*) range.pos:=FALSE; range.min:=0AX; range.max:=range.min;
		NEW(new); new.min:=0DX; new.max:=new.min; range.next:=new;
	ELSE 
			IF (iden="s") OR (iden="S") (* [#x20\t\n\r] *) THEN 
				IF iden="s" THEN range.pos:=TRUE END;
				range.min:=" "; range.max:=range.min;
				NEW(new); new.min:=09X; new.max:=new.min;
				range.next:=new; last:=range.next;
				NEW(new); new.min:=0AX; new.max:=new.min;
				last.next:=new; last:=new;
				NEW(new); new.min:=0DX; new.max:=new.min;
				last.next:=new;
			ELSIF (iden="i") OR (iden="I") THEN
				IF iden="i" THEN range.pos:=TRUE END;
				range.min:=CHR(65);range.max:=CHR(90);
				NEW(new); new.min:=CHR(97); new.max:=CHR(122);
				range.next:=new; last:=range.next;
				NEW(new); new.min:=CHR(192); new.max:=CHR(214);
				last.next:=new; last:=new;
				NEW(new); new.min:=CHR(216); new.max:=CHR(246);
				last.next:=new; last:=new;
				NEW(new); new.min:=CHR(248); new.max:=CHR(255);
				last.next:=new; last:=new;
				NEW(new); new.min:="_"; new.max:=new.min;
				last.next:=new; last:=new;
				NEW(new); new.min:=":"; new.max:=new.min;
				last.next:=new; 
			ELSIF (iden="c") OR (iden="C") THEN
				IF iden="c" THEN range.pos:=TRUE END;
				range.min:=CHR(65);range.max:=CHR(90);
				NEW(new); new.min:=CHR(97); new.max:=CHR(122);
				range.next:=new; last:=range.next;
				NEW(new); new.min:=CHR(192); new.max:=CHR(214);
				last.next:=new; last:=new;
				NEW(new); new.min:=CHR(216); new.max:=CHR(246);
				last.next:=new; last:=new;
				NEW(new); new.min:=CHR(248); new.max:=CHR(255);
				last.next:=new; last:=new;
				NEW(new); new.min:=CHR(48); new.max:=CHR(57);
				last.next:=new; last:=new;
				NEW(new); new.min:="."; new.max:=new.min;
				last.next:=new; last:=new;
				NEW(new); new.min:="-"; new.max:=new.min;
				last.next:=new; last:=new;
				NEW(new); new.min:="_"; new.max:=new.min;
				last.next:=new; last:=new;
				NEW(new); new.min:=":"; new.max:=new.min;
				last.next:=new; 
			END;
	END;
END MultiCharEsc;

PROCEDURE unicode():CHAR;
VAR str:ARRAY 7 OF CHAR;i:INTEGER;
BEGIN
	TextsCP.WriteString("unicode: ");
	str[0]:='\';TextsCP.Write(str[0]);
	str[1]:='u';TextsCP.Write(str[1]);
	FOR i:=2 TO 5 DO
		GetSym;	
		TextsCP.Write(ch);
		CASE ch OF	   	
			'0'..'9',
			'A'..'F':str[i]:=ch;
		ELSE Error(2);
		END;
	END;
	TextsCP.WriteLn;
	GetSym;str[6]:=0X;
	TextsCP.WriteString("unicode str: "+str+" ch: ");
	TextsCP.Write(ch);
	TextsCP.WriteLn;
	
	RETURN texts.Texts.convertUnicode(MKSTR(str));
END unicode;



PROCEDURE SingleCharEsc (VAR char:CHAR); 
BEGIN
	TextsCP.WriteString("SingleCharEsc sym: ");
	TextsCP.WriteInt(sym,2);
	TextsCP.WriteLn;
	TextsCP.WriteString(" iden: ");
	TextsCP.Write(iden);
	TextsCP.WriteLn;
	TextsCP.WriteString(" ch: ");
	TextsCP.Write(ch);
	TextsCP.WriteLn;
	CASE sym OF
	  escape: char:="\";
	  TextsCP.WriteString("SingleCharEsc escape");
	  TextsCP.WriteLn;
	| bar: char:="|";
	| strich: char:="-";
	| osquareBr: char:="[";
	| csquareBr: char:="]";
	| neg: char:="^";
	| lKlammer: char:="(";
	| rKlammer: char:=")";
	| glKlammer: char:="{";
	| grKlammer: char:="}";
	| punkt: char:=".";
	| plus: char:="+";
	| asterisk: char:="*";
	| qum: char:="?";
	ELSE 
		IF iden="n" THEN char:=0AX; 
		ELSIF iden="r" THEN char:=0DX;
		ELSIF iden="t" THEN char:=09X;
		ELSE (* Interface_Halt.halt.HaltPar(127); *) Error(2);
		END;
	END;
	GetSym();
END SingleCharEsc;

PROCEDURE XmlCharRef(VAR cha:CHAR);
VAR zahl,j,res:INTEGER;
BEGIN
IF (sym=char) & (iden="#") THEN GetSym();
	IF (sym=char) THEN 
		IF iden="x" THEN
		(*hex-format: noch nicht implementiert!*)
		ELSIF (iden>="0") & (iden<="9") THEN
			(*"chr"-Format *)
			j:=0;
			WHILE (iden>="0") & (iden<="9") & (sym=char) DO
				digits[j]:=iden; INC (j); GetSym()
			END;
			digits[j]:=0X;
			(*zahl:=BaseStrings.strWrap.StringToInt(digits);*)
			RTS.StrToInt(digits,zahl,ok);
			IF zahl<=255 THEN  (*unterstuetzter Bereich*) 
				cha:=CHR(zahl); 
			ELSE (*Interface_Halt.halt.HaltPar(127)*) Error(6);
			END;
		ELSE (*Interface_Halt.halt.HaltPar(127)*) Error(6);
		END;
	ELSE (* Interface_Halt.halt.HaltPar (127) *) Error(6)
	END;
ELSE (* Interface_Halt.halt.HaltPar (127)*) Error(6)
END;
IF ~error THEN
	IF (sym=char) & (iden=";") THEN GetSym();
	ELSE (*Interface_Halt.halt.HaltPar (127)*)Error(6);
	END;
END
END XmlCharRef;


PROCEDURE quantifier(VAR pie:Piece);
VAR quant:ARRAY 32 OF CHAR;
			j:INTEGER;
			res:INTEGER;
BEGIN
 	NEW (pie.min);
	IF sym=asterisk THEN pie.min.val:=0; 
	pie.id:=3;
	GetSym(); 
	ELSIF sym=plus THEN pie.min.val:=1; 
	pie.id:=3;
	GetSym();
	ELSIF sym=qum THEN pie.min.val:=0;
		NEW(pie.max); pie.max.val:=1; 
		pie.id:=1;
		GetSym();
	ELSIF sym=glKlammer THEN GetSym();
		j:=0;
		WHILE (iden>="0") & (iden<="9") & (sym=char) DO
			digits[j]:=iden; INC (j); GetSym()
		END;
		IF sym=grKlammer THEN (*quantExact*)
			(*pie.min.val:=BaseStrings.strWrap.StringToInt(digits);*)
			RTS.StrToInt(digits,pie.min.val,ok);
			
			NEW(pie.max); pie.max.val:=pie.min.val; 
		(*	pie.matchProc:=MatchProcQuantified;*)
			pie.id:=2;
			GetSym();
		ELSIF iden="," THEN
			(*pie.min.val:=BaseStrings.strWrap.StringToInt(digits);*)
			RTS.StrToInt(digits,pie.min.val,ok);
			(* GPCP RTS.StrToInt*(IN s : ARRAY OF CHAR;OUT i : INTEGER;OUT ok : BOOLEAN);*)
			GetSym();
			IF sym=grKlammer THEN 
			pie.id:=3;
			GetSym();
			ELSE
				j:=0;
				WHILE (iden>="0") & (iden<="9") & (sym=char)  DO
					digits[j]:=iden; INC (j); GetSym()
				END;
				IF sym=grKlammer THEN (*quantExact*)
					NEW(pie.max); 				   						
					(*pie.max.val:=BaseStrings.strWrap.StringToInt(digits);*)
					RTS.StrToInt(digits,pie.max.val,ok);
					(* GPCP RTS.StrToInt*(IN s : ARRAY OF CHAR;OUT i : INTEGER;OUT ok : BOOLEAN);*)
					pie.id:=2; 
					GetSym();
				ELSE (*Interface_Halt.halt.HaltPar(127);*) Error(6);
				END;
			END;
		ELSE (*Interface_Halt.halt.HaltPar(127)*) Error(6);
		END;
	ELSE pie.min.val:=1; NEW(pie.max); pie.max.val:=1; pie.id:=2;
	END;
	IF ~error THEN
		(*quantValid?*)
		IF  ~(pie.max=NIL)  & ~(pie.min.val<=pie.max.val)   THEN
			(*Interface_Halt.halt.HaltPar(127);*) Error(6) END;
	END		
END quantifier;

PROCEDURE^ charClassExpr (VAR ran:Range);
PROCEDURE^ posCharGroup(VAR range:Range);
PROCEDURE^ charClassSub(VAR range:Range);

PROCEDURE charGroup (VAR range:Range);
BEGIN (*charGroup*)
	IF (*sym=neg*) iden="^" THEN range.pos:=FALSE; GetSym(); 
	ELSE range.pos:=TRUE END;
	posCharGroup(range);
END charGroup;

PROCEDURE charClassSub(VAR range:Range);
BEGIN (*charClassSub*)
	GetSym(); (*osquareBr*)
	GetSym();NEW(range.next); range.next.sub:=TRUE;
	charClassExpr(range.next);
END charClassSub;
			
PROCEDURE posCharGroup(VAR range:Range);
VAR i:CHAR;
BEGIN (*posCharGroup*)
	IF iden="&" THEN GetSym(); XmlCharRef(range.min);
		IF ~error THEN
			range.max:=range.min;
		END
	ELSIF (sym=char) OR (sym=escape) THEN						(*OR escape*)
		IF sym=char THEN range.min:=iden; GetSym();
		ELSE 
			TextsCP.WriteString("posCharGroup vor GetSym sym: ");
			TextsCP.WriteInt(sym,2);
			TextsCP.WriteString(" ch: ");
			TextsCP.Write(ch);
			TextsCP.WriteLn;
			IF ch='u' THEN range.min :=unicode();
				TextsCP.WriteString("range.min: ");
				TextsCP.Write(range.min);
				TextsCP.WriteLn;
			ELSE
				GetSym(); 
				TextsCP.WriteString("posCharGroup nach GetSym sym: ");
				TextsCP.WriteInt(sym,2);
				TextsCP.WriteString(" ch: ");
				TextsCP.Write(ch);
				TextsCP.WriteLn;			
				SingleCharEsc(range.min);
			END;
		END;
		IF ~error THEN
			IF (sym=char) OR (sym=escape) THEN					(* OR escape*)
				range.max:=range.min;
				TextsCP.WriteString("range.max: ");
				TextsCP.Write(range.max);
				TextsCP.WriteLn;
			ELSIF sym=strich THEN GetSym();
				IF (sym=char) OR (sym=escape) THEN				(*OR  escape*)
					IF sym=char THEN range.max:=iden; GetSym();
					ELSE 
						(* nach escape *)
						TextsCP.WriteString("posCharGroup vor GetSym Strich escape sym: ");
						TextsCP.WriteInt(sym,2);
						TextsCP.WriteString(" ch: ");
						TextsCP.Write(ch);
						TextsCP.WriteLn;
						
						(* escape *)
						TextsCP.WriteString("posCharGroup nach GetSym Strich escape sym: ");
						TextsCP.WriteInt(sym,2);
						TextsCP.WriteString(" ch: ");
						TextsCP.Write(ch);
						TextsCP.WriteLn;
						IF ch='u' THEN 
							range.max:=unicode();
							TextsCP.WriteString("range.max: ");
							TextsCP.Write(range.max);
							TextsCP.WriteLn;
							TextsCP.WriteString("sym");
							TextsCP.WriteInt(sym,2);
							TextsCP.WriteLn;
						ELSE
						
							SingleCharEsc(range.max);
						END;
					END;
				ELSIF sym=osquareBr THEN range.max:=range.min; GetSym(); 
					NEW(range.next);range.next.sub:=TRUE; charClassExpr(range.next);
				ELSE (* Interface_Halt.halt.HaltPar(127)*) Error(6);
				END;
			ELSIF sym=csquareBr THEN 
				range.max:=range.min;
			ELSE (* Interface_Halt.halt.HaltPar(127); *) Error(5);
			END;
		END;
	ELSE (* Interface_Halt.halt.HaltPar (127); *) Error(5);
	END;
		
	(*validRange?*)
	IF ~error THEN
		IF ~(range.min<=range.max) THEN (* Interface_Halt.halt.HaltPar(127)*)Error(6) END;
		IF ~error THEN	
			IF sym=strich THEN charClassSub (range) END;
		END;
	END
	END posCharGroup;

PROCEDURE charClassExpr (VAR ran:Range);
VAR new,last:Range; 
BEGIN (*charClassExpr*)
	charGroup(ran);
	IF ~error THEN
	 	last:=ran;
		WHILE (~error) & (sym=char) DO 
			NEW(new); posCharGroup (new); 
			IF ~error THEN
				last.next:=new;
				last:=new;
			END
			END;
		IF ~error THEN
			IF sym=csquareBr THEN GetSym();
			ELSE (* Interface_Halt.halt.HaltPar(127); *) Error(4);
			END;
		END
	END;
END charClassExpr;

PROCEDURE (reg:Regex) regExp(),NEW;
VAR last,new:Branch;

	PROCEDURE branch(VAR bra:Branch);
	VAR last:Piece;
			new:Piece;
	
		PROCEDURE piece(VAR pie:Piece);
		
			PROCEDURE atom(VAR pie:Piece);
			VAR ato:Atom;
					ran:Range;
			
			BEGIN (*atom*)
			NEW(pie.atom); ato:=pie.atom;
			
			
			IF sym=char THEN
				NEW(ato.range); ran:=ato.range;
				ran.min:=iden; ran.max:=ran.min;ran.pos:=TRUE;
				GetSym();
				
			ELSIF sym=escape THEN
				NEW(ato.range); ran:=ato.range;ran.pos:=TRUE;
				GetSym();
				MultiCharEsc(ran);
				IF ran.min=0X THEN
					TextsCP.WriteString("Atom sym: ");
					TextsCP.WriteInt(sym,2);
					TextsCP.WriteString(" ch: ");
					TextsCP.Write(ch);
					TextsCP.WriteLn;
					(* char before *)
					IF iden ='u' THEN
						(* first digit of unicode here
						already read, hence decree global
						index i (dirty!) of regex string *)
						DEC(i);
						ran.min:=unicode();
						TextsCP.WriteString("unicode range.min: ");
						TextsCP.Write(ran.min);
						TextsCP.WriteLn;
						sym:=other (* other, not escape *)
					ELSE
						SingleCharEsc(ran.min);
					END;
					IF ~error THEN
						ran.max:=ran.min;
					ELSE TextsCP.WriteString("error");
						TextsCP.WriteLn;
					END
				ELSE iden:=0X;GetSym();
				END;
				
			ELSIF sym=punkt THEN
				NEW(ato.range); ran:=ato.range;
				MultiCharEsc(ran);
				GetSym();
				
			ELSIF sym=osquareBr THEN
				GetSym();
				NEW(ato.range); ran:=ato.range;
				charClassExpr(ran);
				
			ELSIF sym=lKlammer THEN GetSym();
				NEW(ato.regex); ato.regex.regExp();
				IF ~error THEN
					IF sym=rKlammer THEN GetSym();
					ELSE (* Interface_Halt.halt.HaltPar(127) *) Error(3);
					END;
				END
				
			ELSE (* Interface_Halt.halt.HaltPar(127) *) Error(1); 
			END;
			END atom;
		
		BEGIN (*piece*)
		 	atom(pie);
			IF ~error THEN   
				quantifier(pie);  
			END;
		END piece;
		
	BEGIN (*branch*)
	NEW (bra.piece); piece(bra.piece); (*first*)
		last:=bra.piece;
	IF ~error THEN   
		WHILE (~error) & ((sym=char) OR (sym=osquareBr) OR (sym=lKlammer) OR (sym=escape) OR (sym=punkt)) DO
			NEW (new); piece(new);
			IF ~error THEN
				TextsCP.WriteInt(sym,2);
				TextsCP.WriteStringLn("piece suc gesetzt");
				last.suc:=new;
				last:=new;
			END
		END;
	globalLast:=last;
	END;
	END branch;
	
BEGIN (*regExp*)
NEW(reg.branch); branch(reg.branch); (*first*)
reg.last:=reg.branch; (*q*)
IF ~error THEN   
WHILE (~error) & (sym=bar) DO 
	GetSym();
	NEW(new); branch(new);
	IF ~error THEN
		reg.last.alt:=new;
		reg.last:=new;
	END;
END;
END;
END regExp;

PROCEDURE InitCreateRegex*(str:POINTER TO ARRAY OF CHAR;VAR reg:Regex);
VAR termPiece:Piece; quant:Quantifier;
BEGIN
	error:=FALSE;
	regString:=str;i:=0;Read();GetSym();
	reg.regExp();
	(*Hier an RegExpr. noch ein piece.range.min:=0X haengen*)
	NEW(termPiece);
	NEW(termPiece.atom);
	NEW(termPiece.atom.range);
	termPiece.atom.range.min:=0X;termPiece.atom.range.max:=0X;
	termPiece.atom.range.pos:=TRUE;
	NEW(quant);quant.val:=1;
	NEW(termPiece.min);
	termPiece.min:=quant;
	NEW(termPiece.max);
	termPiece.max:=quant;
	termPiece.id:=2;
	globalLast.suc:=termPiece;
	reg.lastPiece:=globalLast;
	reg.Font:=NIL;	
END InitCreateRegex;


END RegexParser.