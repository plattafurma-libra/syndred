package syndred.logic;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutionException;

import org.springframework.http.ResponseEntity;
import org.springframework.messaging.handler.annotation.DestinationVariable;
import org.springframework.messaging.handler.annotation.MessageMapping;
import org.springframework.messaging.handler.annotation.SendTo;
import org.springframework.messaging.simp.annotation.SubscribeMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.multipart.MultipartFile;

import syndred.entities.Block;
import syndred.entities.Data;
import syndred.entities.EntityMap;
import syndred.entities.InlineStyleRange;
import syndred.entities.Parser;
import syndred.entities.RawDraftContentState;

@org.springframework.stereotype.Controller
public class Controller {

	@SubscribeMapping("/{instance}/editor/pull")
	public RawDraftContentState editorPull(@DestinationVariable String instance) {
		RawDraftContentState state = Threading.pull(instance);
		
		// TODO success-Meldung statt Baum, da Download von Server
		if ((state.isGenerator() != null) && state.isGenerator().equals("true")) {
			String tree = state.getParseTree();
			state = new RawDraftContentState();
			state.setParseTree(tree);
		}
		
		return state;
	}

	@MessageMapping("/{instance}/editor/push")
	public void editorPush(@DestinationVariable String instance, RawDraftContentState state)
			throws InterruptedException {
		Threading.push(instance, state);
	}

	@SubscribeMapping("/{instance}/parser/pull")
	public Parser parserPull(@DestinationVariable String instance) {
		return Threading.parser(instance);
	}

	@MessageMapping("/{instance}/parser/push")
	@SendTo("/syndred/{instance}/parser/pull")
	public Parser parserPush(@DestinationVariable String instance, Parser parser)
			throws InterruptedException, ExecutionException {
		return Threading.run(instance, parser);
	}
	
	// TODO saving on server
	@MessageMapping("/{instance}/parser/upload")
	public void parserUpload(@DestinationVariable String instance, String grammar)
			throws InterruptedException, IOException {
		File dir = new File("vendor/tmp");
		File[] files = dir.listFiles();
		
		if (files.length > 0) {
			for (File file : files) 
				file.delete();
		}
		
		File file = new File(dir, instance.replace("#", "").concat(".txt"));
		
		FileOutputStream fos = new FileOutputStream(file);
		OutputStreamWriter osw = new OutputStreamWriter(fos, "UTF-8");
		BufferedWriter bfw = new BufferedWriter(osw);
		
		bfw.write(grammar);
		bfw.close();
	}
	
	
	
	// TODO
	@RequestMapping(method = RequestMethod.POST, path = "/generator/files")
    public ResponseEntity<?> handleFileUpload(@RequestParam("file") MultipartFile mFile, @RequestParam("instance") String instance) {
		 System.err.println(mFile.getOriginalFilename() + " - " + mFile.getSize() + " - " + instance);
		
		try {
			InputStream in = new ByteArrayInputStream(mFile.getBytes());
			InputStreamReader isr = new InputStreamReader(in);
			BufferedReader bfr = new BufferedReader(isr);
			
			String curLine;
			
			RawDraftContentState state = new RawDraftContentState();
			state.setGenerator("true");
			
			List<Block> blocks = new ArrayList<Block>();
			
			while ((curLine = bfr.readLine()) != null) {
				Block block = new Block();
				block.setKey(String.valueOf(blocks.size()));
				block.setText(curLine);
				block.setType("unstyled");
				block.setDepth(0);
				block.setInlineStyleRanges(new ArrayList<InlineStyleRange>());
				block.setEntityRanges(new ArrayList<Object>());
				block.setData(new Data());
				
				blocks.add(block);
			}
			
			bfr.close();
			
			state.setBlocks(blocks);
			state.setEntityMap(new EntityMap());
			
//			ObjectMapper mapper = new ObjectMapper();
//			String stateStr = mapper.writeValueAsString(state);
//			
//			System.err.println(stateStr);
			
			Threading.push(instance, state);
		} catch (IOException e) {
			e.printStackTrace();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		
		
//		File dir = new File("vendor/tmp");
//		File[] files = dir.listFiles();
//		
//		if (files.length > 0) {
//			for (File file : files) 
//				file.delete();
//		}
//		
//		if (mFile.getSize() > 0) {
//            File fileToSave = new File(dir, mFile.getOriginalFilename());
// 			
//            try {
//	            	BufferedOutputStream stream = new BufferedOutputStream(new FileOutputStream(fileToSave));
//	    			stream.write(mFile.getBytes());
//	    			stream.close();
//    			
//	    			System.err.println("saved.");
//    			
//    			
//            
//	            FileInputStream fis = new FileInputStream(fileToSave);
//	            InputStreamReader isr = new InputStreamReader(fis);
//	            BufferedReader bfr = new BufferedReader(isr);
//	            
//	            String curLine;
//	            StringBuffer sb = new StringBuffer();
//	            
//	            while ((curLine = bfr.readLine()) != null) {
//	            		sb.append(curLine);
//	            }
//	            
//	            String content = sb.toString();
//	            System.err.println(/*content.length()*/"read.");
//	            
//	            // TODO RawDraftContentState...
//	            
//	            
//	            
//	            bfr.close();
//            } catch (IOException e) {
//            		return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).build();
//            }
//        } else
//        		return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).build();

        return ResponseEntity.ok().build();
    }

}
