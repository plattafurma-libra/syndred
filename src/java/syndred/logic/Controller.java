package syndred.logic;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.concurrent.ExecutionException;

import org.springframework.messaging.handler.annotation.DestinationVariable;
import org.springframework.messaging.handler.annotation.MessageMapping;
import org.springframework.messaging.handler.annotation.SendTo;
import org.springframework.messaging.simp.annotation.SubscribeMapping;

import syndred.entities.Parser;
import syndred.entities.RawDraftContentState;

@org.springframework.stereotype.Controller
public class Controller {

	@SubscribeMapping("/{instance}/editor/pull")
	public RawDraftContentState editorPull(@DestinationVariable String instance) {
		return Threading.pull(instance);
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

}
