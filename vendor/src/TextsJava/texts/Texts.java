package texts;

import java.io.BufferedReader;
import java.io.FileReader;
import java.math.BigInteger;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List; 

/* 
 * Texts contains an array textAsCharSeq for storing characters entered by the editor.
 * textAsCharSeq is a private. It is accessed by getters and setters, e.g. for parsing.
 * Texts is part (attribute) of the Shared class by which thread communication is done
 */

public class Texts {

	BufferedReader in=null;
	public boolean eot;
	private List<RichChar> textAsRichCharList;
	//private char[] textAsCharSeq;
//	private int textLen;
	private int parsePos;
	//-----------if ok print in console--------
	public boolean ok=true;
	//-----------syntaxDriven true syndred false-------
	public static boolean syntaxDriven=false;
	//-----------------------------------------
	static private String grammar;
	static private String reservedGrammar="vendor/res/tmp/ReservedGrammar.txt";	// TODO server path, see below
	//static public String regex="C://users//rols//Regex.txt";
	private StringBuffer ebnfBuf=new StringBuffer();
	private int ebnfPos=0;
	public boolean ebnfEot=false;
	// cstr
	public Texts() {
		this.textAsRichCharList=new /*Array*/LinkedList<RichChar>();//new char[1000000];
//		textLen=0;
		parsePos=0;
	}
	
	
	/*public char[] getTextAsCharArray() {
		return this.textAsCharSeq;
	}
	
	*/
	
	// TODO server path, see above
	protected void setGrammar(String grammar) {
		String hash = "#";
		
		if (grammar.startsWith(hash)) {
			grammar = grammar.replace(hash, "");
			Texts.grammar = "vendor/tmp/";
		} else
			Texts.grammar = "vendor/res/tmp/";
		
		Texts.grammar = Texts.grammar.concat(grammar);
	}
	
	public void setTextAsRichCharList(String str) {
		this.textAsRichCharList = 
			new ArrayList<RichChar>();
				//str.toCharArray();
		for (int i=0;i<str.length();i++) {
			RichChar rChar=new RichChar(str.charAt(i));
			//18-11-21 rChar.ch=str.charAt(i);
			this.textAsRichCharList.add(rChar);
		}
//		this.textLen=str.length();
	}
		
		
	public int getTextLen(){
//		return this.textLen;
		return textAsRichCharList.size();
	}
	
	
	public int getParsePos(){
		return parsePos;
	}
	
	public void setParsePos(int pos){
		this.parsePos=pos;
	}
	
	public void incParsePos(){
		this.parsePos++;
	}

	public RichChar getRichChar() {
		return textAsRichCharList.get(parsePos++);
	}
	
	/*public char getTextChar(){
		//todo error if textPos>= textLen
		char ch=this.textAsCharSeq[parsePos];
		parsePos++;
		return ch;
	}*/
	
	public char getTextCharAtPos(int pos){
		RichChar rChar;
		rChar=this.textAsRichCharList.get(pos);
		if (ok) System.out.println("texts.Text.getTextCharAtPos pos: "+pos+ " ch:"+
		rChar.ch);
		//this.textAsCharSeq[pos]);
		if (pos >=/*this.textLen*/getTextLen()) 
			//toDo
			{int i = 10/0;return 0;}
		else return //this.textAsCharSeq[pos];
				rChar.ch;
	}
	
	public void setTextCharAtPos(char ch, int pos) {
		try {
			if (pos > /*this.textLen*/getTextLen()) 
				//
				{throw new Exception
					("setTextCharAtPod pos>textLen");
			
				}
			else {RichChar rChar=new RichChar(ch);
				// 18-11-21 rChar.ch=ch;
				this.textAsRichCharList.set(pos,rChar);//[pos]=ch;
			}
		}
			catch (Exception e) {
				System.out.println
				("error setTextCharAtPod pos>textLen");
				e.printStackTrace();
			}
		
		
	}
		
	public void open(String fileName){
	if (ok) System.out.println("Texts.open: "+fileName);
	this.eot=false;
	try { 
		// EbnfGrammar is found in eclipse 
		in = new BufferedReader(new FileReader(fileName));
		if (ok) System.out.println(fileName.toString()+" opened");
		}
			catch(Exception e){
			if (ok) System.out.println(fileName.toString()+" not found");
		}
		
	}//open
	
	public void openEbnf()	{
		try { 
			char ch;
			// EbnfGrammar is found in eclipse 
			in = new BufferedReader(new FileReader(grammar));
			if (ok) System.out.println(grammar+" opened");
			this.eot=false;
			
			while (true) {
				ch=readCharFromFile();
				if (this.eot) 
					{ebnfBuf.append(System.lineSeparator());
					break;
					}
				else ebnfBuf.append(ch);
			}
					
			
			in = new BufferedReader(new FileReader(reservedGrammar));
			if (ok) System.out.println(reservedGrammar+" opened");
			this.eot=false;
			while (true) {
				ch=readCharFromFile();
				if (this.eot)break;
				ebnfBuf.append(ch);
			}
		}
			catch(Exception e){
			System.out.println(grammar+" or "+reservedGrammar +" not found");
			e.printStackTrace();
		}
		System.out.print(ebnfBuf);
			
		
	}
	
	public char readEbnfChar() {
		try {
			if (ebnfEot) new Exception("ebnfEot false");
			if (ebnfPos>=ebnfBuf.length()) {
				ebnfEot=true;
				return ' ';
			}
			else {
				char ch=ebnfBuf.charAt(ebnfPos);
				ebnfPos++;
				System.out.println("readEbnfChar: "+ch);
				return ch;
				}
		}
			catch(Exception e){
				System.out.println("ebnfEot false");
				e.printStackTrace();
				return ' ';
			}
	}
		
	public char readCharFromFile(){
		
		int val;	
		try {
			val=in.read();			
			if (val==-1) {this.eot=true;return ' ';}
			else return (char)val;
			}
		catch(Exception e){
			if (ok) System.out.println("read error");
			}
		eot=true;
		return ' ';
	}//read
	
	
	
	/*
	public void readTextFromFile(String filename){
		if (ok) System.out.println("Texts.readText: "+filename);
		this.open(filename);
		while(true){
			char ch=this.readCharFromFile();
			if (ok) System.out.print(ch);;
			if (this.eot)break;
			this.textAsCharSeq[textLen]=ch;
			textLen++;
		}
		if (ok) System.out.println();
	}//readText	
	*/
	
	/*******Only for conversion of longint for Syntax Driven Editor Console Writing *****
		public static char[] convertLongIntToCharArray
		(long l){
			// to string
			String str=Long.toString(l);
			char[] charArray = str.toCharArray();
			return charArray;
		}
***********************/
		// only called from RegexReplace.cp
		// SDE
		/*public static char convertUnicode(String s) {
	        if (s.startsWith("\\u")) {
	            String numberStr = s.substring(2);
	            
	            int i= (int) new BigInteger(numberStr, 16).intValue();
	            System.out.println("BigInteger: "+i);
	            char ch= (char)i;
	            System.out.println("ch : "+ch);
	            char c = (char) new BigInteger(numberStr, 16).intValue();
	            return c;//new Character(c).toString();
	        }
	        return 0;
	    }*/
	
		// syndred
		public static char convertUnicode(String s) {
			return s.startsWith("\\u") ? (char) new BigInteger(s.substring(2), 16).intValue() : 0;
		}
	
		
		
	public List<RichChar> getRichChars() {
		return textAsRichCharList;
	}
	
	public void setRichChars(List<RichChar> chars) {
		textAsRichCharList = chars;
	}

}
