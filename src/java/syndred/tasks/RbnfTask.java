package syndred.tasks;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.text.ParseException;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ExecutionException;
import java.util.function.Function;

import org.apache.commons.io.output.TeeOutputStream;

import CP.Ebnf1.Ebnf1_EBNF;
import syndred.entities.Block;
import syndred.entities.InlineStyleRange;
import syndred.entities.Parser;
import syndred.entities.RawDraftContentState;
import syndred.logic.DraftState;
import texts.RichChar;
import texts.Shared;
import texts.Texts;
import tree.Node;

public class RbnfTask extends Task {

		private Thread ebnfThread;
	
		private Shared shared;
	
		private Ebnf1_EBNF ebnf;
		
		private boolean success;

	public RbnfTask(BlockingQueue<RawDraftContentState> input, Function<RawDraftContentState, Exception> output,
			Parser parser) throws ExecutionException {
		super(input, output, parser);

		ebnfThread = new Thread(() -> {
			PrintStream consoleStream = System.out;
			OutputStream fileStream = null;
			
			try {
				fileStream = /*new BufferedOutputStream(*/new FileOutputStream("vendor/log/EBNF.log")/*)*/;
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			}

			TeeOutputStream dualStream = new TeeOutputStream(fileStream, consoleStream);
			PrintStream log = new PrintStream(dualStream/*fileStream*/);
			
			System.setOut(log);
			
			ebnf = new Ebnf1_EBNF();	
			ebnf.init(shared);
			
			while (!Thread.interrupted()) {
//				System.setOut(log);
				
				try {
					ebnf.root = ebnf.syntaxDrivenParse();					
					success = true;

					while (success) {
//						System.setOut(consoleStream);
						Thread.sleep(100);
					}
				} catch (Throwable thrown) {
				}
			} 
		});

		shared = new Shared();
		shared.setGrammar(parser.getGrammar());

		try {
			ebnfThread.start();
		} catch (Throwable thrown) {
			throw new ExecutionException(thrown);
		}
	}

	@Override
	public void close() {
		while (!ebnfThread.isInterrupted() || ebnfThread.isAlive())
			ebnfThread.interrupt();
	}

	@Override
	protected RawDraftContentState parse(RawDraftContentState state) throws ParseException {
		DraftState.del(state, "Error");
		DraftState.del(state, "Success");

		ebnf.iNode.reset();
		
		Texts sharedText = shared.getSharedText();
		List<RichChar> next = getRichChars(state);
		List<RichChar> prev = sharedText.getRichChars();
		int position = 0;

		try {
			while (next.get(position).equals(prev.get(position)))
				position++;
		} catch (IndexOutOfBoundsException e) {
		}

		shared.maxPosInParse = -1;
		sharedText.setParsePos(0);
		sharedText.setRichChars(next);
		success = false;

		try {
			while (!success && shared.maxPosInParse < 0 && sharedText.getParsePos() < sharedText.getTextLen())
				Thread.sleep(100);
		} catch (InterruptedException e) {
			return state;
		}

		String parseTree = null;
		
		if (shared.maxPosInParse >= 0) {
			DraftState.add(state, "Error", shared.maxPosInParse, next.size() - shared.maxPosInParse);
		} else if (success) {
			DraftState.add(state, "Success", 0, next.size());
			parseTree = Node.resultString;
		} 
		
		if (state.isGenerator() == null)
			state.setGenerator("false");
		
		state.setParseTree(parseTree);
		return state;
	}

	private List<RichChar> getRichChars(RawDraftContentState state) {
		List<RichChar> chars = new LinkedList<RichChar>();

		for (Block block : state.getBlocks()) {
			String text = block.getText();

			for (int i = 0; i < text.length(); i++) {
				RichChar rch = new RichChar(text.charAt(i));

				for (InlineStyleRange range : block.getInlineStyleRanges()) {
					if (range.getOffset() > i || range.getOffset() + range.getLength() <= i)
						continue;

					char[] style = range.getStyle().toCharArray();

					if (!range.getStyle().matches("\\d+"))
						switch (range.getStyle()) {
						case "Red":
						case "Yellow":
						case "Green":
						case "Blue":
							rch.color = style;
							break;

						case "Italic":
						case "Underline":
							rch.style = style;
							break;

						case "Arial":
						case "Courier":
						case "Times":
							rch.typeface = style;
							break;

						case "Bold":
							rch.weight = style;
							break;
						}
					else
						rch.size = style;
				}

				chars.add(rch);
			}
		}

		return chars;
	}
}
