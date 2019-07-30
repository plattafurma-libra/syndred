import React from 'react';
import $ from 'jquery';
import axios from 'axios';

export default class Generator extends React.Component {
	
	constructor(props) {
		super(props);
		this.dest = `${window.dest}/generator/`;
	}
	
	/*componentDidMount() {}*/
	
	// TODO
	render() {
		return (
			<form className='form-horizontal well'>
				<fieldset>
					<legend>Generator</legend>
					
					<div className='form-group'>
						<div>
							<label className='col-lg-2 control-label'>
								Upload
							</label>
						</div>
						<div className='col-lg-10'>
							<input 
								type='file'
								accept='.txt,.json'
								onChange={e => this.handleUploadFile(e.target.files[0])/*this.props.syndred.read(e.target.files[0], this, this.parse)*/} 
								/*disabled*//>
						</div>
					</div>				
					
					<hr/>
					
					<div className='form-group'>
						{/*<label className='col-lg-2'>
							Download
						</label>*/}
						<div className='col-lg-12 text-center'>
							<button
								className='btn btn-primary' /*disabled*/
								onClick={(event) => this.download(event)}>
								Download
							</button>
						</div>
					</div>
				</fieldset>
			</form>);
	}
	
	// TODO
	uploadFileToServer(data){
		let serviceInstance = axios.create({
	        baseURL: 'http://localhost:8080/',	// TODO url, see below (post)
	        timeout: 5000						// TODO required?
	      });
		return serviceInstance.post('/generator/files', data);
    }
	
	// TODO				
	handleUploadFile(file) {
		const data = new FormData();
	    data.append('file', file);
	    data.append('instance', `${location.hash}`);		// TODO tmp, see above (url)
	    
	    this.uploadFileToServer(data).then((response) => {
	        console.log("File " + file.name + " is uploaded");
	    }).catch(function (error) {
	        console.log(error);
	        /*if (error.response) console.log("Upload error. HTTP error/status code=",error.response.status);
	        else console.log("Upload error. HTTP error/status code=",error.message);*/
	    });
	}
					
	download(event) {
		event.preventDefault();
		
		// TODO download JSON
		console.log(JSON.stringify(this.props.tree()));
	}
	
	
	
	
	// TODO delete all below?
	parse(instance, content) {
		let rawDraftContentState = instance.getRawDraftContentState(content);
		//console.log(rawDraftContentState);
		
		instance.props.socket.send(`${window.dest}/editor/push`, { }, rawDraftContentState);
	}
	
	getRawDraftContentState(content) {
		return "{" + this.getState("true") + "," + this.getBlocks(content) + "," + this.getEntityMap("{}") + "}";
	}
	
	getState(value) {
		return "\"generator\":" + this.getString(value);
	}
	
	getEntityMap(value) {
		return "\"entityMap\":" + value;
	}

	getBlocks(content) {
		let blocks = "\"blocks\":[";
		let arr = content.split("\n");
		
		for (var i = 0; i < arr.length; i++) {
			blocks = blocks.concat(this.getBlock(i, arr[i]));
			
			if (i < arr.length-1)
				blocks = blocks.concat(",");
		}
		
		return blocks.concat("]");
	}
	
	getBlock(key, text) {
		return "{" + this.getKey(key) + "," + this.getText(text) + "," + this.getType("unstyled") + ","
			+ this.getDepth(0) + "," + this.getInlineStyleRanges("[]") + "," + this.getEntityRanges("[]") + ","
			+ this.getData("{}") + "}";
	}
	
	getKey(value) {
		return "\"key\":" + this.getString(value);
	}
	
	getText(value) {
		value = value.replace(/"/g, "\\\"");
		return "\"text\":" + this.getString(value);
	}
	
	getType(value) {
		return "\"type\":" + this.getString(value);
	}
	
	getDepth(value) {
		return "\"depth\":" + value;
	}
	
	getInlineStyleRanges(value) {
		return "\"inlineStyleRanges\":" + value;
	}
	
	getEntityRanges(value) {
		return "\"entityRanges\":" + value;
	}
	
	getData(value) {
		return "\"data\":" + value;
	}
	
	getString(value) {
		return "\"" + value + "\"";
	}

}