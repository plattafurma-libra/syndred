import Draft from 'draft-js';
import React from 'react';
import ReactDOM from 'react-dom';
import screenfull from 'screenfull';

import { debounce } from 'lodash';

import Parsetree from './parsetree';

export default class Editor extends React.Component {

	get colorMap() {
		return {
			'Red': { color: 'red' },
			'Yellow': { color: 'yellow '},
			'Green': { color: 'green' },
			'Blue': { color: 'blue' }
		};
	}

	get fontMap() {
		return {
			'Arial': {fontFamily: 'Arial, Helvetica, sans-serif' },
			'Courier': { fontFamily: '"Courier New", Courier, monospace' },
			'Times': { fontFamily: '"Times New Roman", Times, serif' }
		};
	}

	get parseMap() {
		return {
			'Error': { backgroundColor: 'rgba(255, 0, 0, 0.1)' },
			'Success': { backgroundColor: 'rgba(0, 255, 0, 0.1)' }
		};
	}

	get sizeMap() {
		return {
			'6': { fontSize: '6pt' },
			'8': { fontSize: '8pt' },
			'10': { fontSize: '10pt' },
			'12': { fontSize: '12pt' },
			'14': { fontSize: '14pt' },
			'16': { fontSize: '16pt' },
			'20': { fontSize: '20pt' },
			'24': { fontSize: '24pt' }
		};
	}

	get styleMap() {
		return {
			'Bold': { fontWeight: 'bold' },
			'Italic': { fontStyle: 'italic' },
			'Underline': { textDecoration: 'underline' }
		};
	}

	constructor(props) {
		super(props);

		this.dest = `${window.dest}/editor/`;
		this.parse = debounce(this.parse, 500);
		this.state = {
			bounds: null,
			editor: Draft.EditorState.createEmpty(),
			fullscreen: false,
			treeData: null,
			treeView: false
		};
	}

	componentDidMount() {
		window.addEventListener('resize', () => this.forceUpdate());

		screenfull.onchange(
			() => this.setState({ fullscreen: screenfull.isFullscreen }));

		this.props.socket.subscribe(`${this.dest}pull`,
			(message) => {
				let data = JSON.parse(message.body);
				if (Object.getOwnPropertyNames(data).length == 0) return;
				
				let cursor = this.state.editor.getSelection();
				let editor = Draft.EditorState
					.push(this.state.editor, Draft.convertFromRaw(data));

				if (cursor.getHasFocus())
					editor = Draft.EditorState.forceSelection(editor, cursor);

				//console.log('parseTree', data.parseTree);
				console.log('currentInlineStyle', editor.getCurrentInlineStyle()); /* TODO OrderedSet get Success/Error */
				let treeData = data.parseTree
					? JSON.parse(data.parseTree) : null;
			
				this.setState({ editor, treeData, treeView: false },
					() => this.screen.classList.remove('disabled'));
			}
		);
	}

    componentWillUnmount() {
        window.removeEventListener('resize', () => this.forceUpdate());
    }

	cursor() {
		let editor = this.state.editor;
		return Draft.EditorState.forceSelection(editor, editor.getSelection());
	}

	draft(editor) {
		let parse = !this.state.editor.getCurrentContent()
			.equals(editor.getCurrentContent());

		this.setState({ editor }, () => parse && this.parse());
	}

	parse() {
		let data = Draft.convertToRaw(this.state.editor.getCurrentContent());

		this.screen.classList.add('disabled');
		this.props.socket.send(`${this.dest}push`, { }, JSON.stringify(data));
	}

	range(name, map) {
		let markup = this.state.editor.getCurrentInlineStyle();
		let state = this.cursor();

		if (map) Object.keys(map).forEach((i) => state = markup.has(i)
			? Draft.RichUtils.toggleInlineStyle(state, i) : state);

		this.editor.focus();
		this.draft(Draft.RichUtils.toggleInlineStyle(state, name));
	}

	render() {
		let height = (top) => {
			let height = window.innerHeight - top - 25;
			return `${height >= 400 ? height : 400}px`;
		};

		return (
			<div className='well flex-column disabled'
				ref={(element) => this.screen = ReactDOM.findDOMNode(element)}>
				<div className='editor-menu'>
					{this.renderFullscreen()}	
					{this.renderInput()}
					{this.renderStyles()}
					{this.renderSelect('Size', this.sizeMap)}
					{this.renderSelect('Font', this.fontMap)}
					{this.renderSelect('Color', this.colorMap)}
					{this.renderTree()}
				</div>
				<div className='editor-root'
					ref={(element) => element && Object.assign(element.style, {
						height: height(element.getBoundingClientRect().top)
					})}>
					<Draft.Editor
						customStyleMap={Object.assign({ },
							this.colorMap,
							this.fontMap,
							this.parseMap,
							this.sizeMap,
							this.styleMap
						)}
						editorState={this.state.editor}
						onChange={(state) => this.draft(state)}
						ref={(element) => this.editor = element}
						spellCheck={false}
					/>
					{this.state.treeData && this.state.treeView && (
						<Parsetree
							canvas={this.editor.editor.getBoundingClientRect()}
							treeData={this.state.treeData}
						/>
					)}
				</div>
			</div>
		);
	}
	
	renderFullscreen() {
		if (!screenfull.enabled) return null;
		let active = this.state.fullscreen;
		let toggle = () => {
			screenfull.toggle(this.screen);
			this.setState({ editor: this.cursor() }, () => this.editor.focus());
		};

		return (
			<button className={`btn btn-${active ? 'primary' : 'default'}`}
				onClick={() => toggle()}>
				<i className={`fa fa-${active ? 'compress' : 'expand'}`} />
			</button>
		);
	}

	renderSelect(label, map) {
		let markup = this.state.editor.getCurrentInlineStyle();
		let value = Object.keys(map).find((i) => markup.has(i)) || '';

		return (
			<select className='form-control inline input-lg'
				onChange={(event) => this.range(event.target.value, map)}
				value={value}>
				<option value=''>{label}</option>
				{Object.keys(map).map((i) => (<option key={i}>{i}</option>))}
			</select>
		);
	}

	renderStyles() {
		let markup = this.state.editor.getCurrentInlineStyle();
		let style = (s) => `btn btn-${markup.has(s) ? 'primary' : 'default'}`;

		return (
			<div className='btn-group'>
				{Object.keys(this.styleMap).map((i) => (
					<button key={i} className={style(i)}
						onClick={() => this.range(i)}>
						<i className={`fa fa-${i.toLowerCase()}`} />
					</button>
				))}
			</div>
		);
	}

	renderTree() {
		let active = this.state.treeData && this.state.treeView;
		let toggle = () => this.setState({ treeView: !this.state.treeView });

		return (
			<button className={`btn btn-${active ? 'primary' : 'default'}`}
				disabled={!this.state.treeData} onClick={() => toggle()}>
				<i className='fa fa-code-fork' />
			</button>
		);
	}
	
	handleFile(file) {		
		let reader = new FileReader();
		reader.onloadend = (e) => {
			let content = e.target.result;
			
			let contentState = Draft.ContentState.createFromText(content);
			let editorState = Draft.EditorState.createWithContent(contentState);
			
			// TODO clear parsetree?
			// TODO focus if parsetree
			this.setState({ /*treeData: null, treeView: false,*/ editor : editorState }, () => this.parse());
		};
		reader.readAsText(file);
	}
	
	renderInput() {		
		return (
			<label className='btn btn-default'>
				<i className='fa fa-paste' />
				<input 
					type='file'
					accept='.txt'
					style={{display: 'none'}}
					onChange={e => this.handleFile(e.target.files[0])} />
					{/* TODO onChange? */}
			</label>	
		);
	}

}
