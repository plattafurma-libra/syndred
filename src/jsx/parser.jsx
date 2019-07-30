import React from 'react';
import Textarea from 'react-textarea-autosize';
import $ from 'jquery';

export default class Parser extends React.Component {

	/*get parserMap() {
		return {
			abnf: 'Augmented Backus–Naur form (ABNF)',
			rbnf: 'Rich Backus–Naur form (RBNF)',
			regex: 'Regular Expressions',
			test: 'Simple echo test'
		};
	}*/
	
	get grammarMap() {
		return {
			// TODO tmp: complete
			testLexGrammar: 'testLexGrammar',
			testLexGrammar3: 'testLexGrammar3'
		};
	}

	constructor(props) {
		super(props);

		this.dest = `${window.dest}/parser/`;
		this.state = {
			name: '',
			grammar: '',
			error: '',
			running: false
		};
	}

	componentDidMount() {
		this.props.socket.subscribe(`${this.dest}pull`,
			(message) => this.setState(JSON.parse(message.body),
				() => this.props.run()));

		this.setListeners();
	}
	
	setParser(event) {
		event.preventDefault();

		this.initGrammar();	
				
		this.props.socket
			.send(`${this.dest}push`, { }, JSON.stringify(this.state));
	}

	render() {
		return (
			<form id='parser' className='form-horizontal well'>
				<fieldset>
					<legend>Parser</legend>
					{this.state.error && this.renderError()}

					{/*GRAMMAR*/}
					<div className='form-group'>
						<div className='col-lg-10 col-lg-offset-2 text-center'>
							<a href='#grammar'>
								<span className='glyphicon glyphicon-chevron-up'></span>
							</a>
						</div>
						<div className={(this.state.error ? ' has-error' : '') 
							+ (this.state.running ?' has-success' : '')}>
							<label className='col-lg-2 control-label'>
								Grammar
							</label>
						</div>
						<div className='col-lg-10 collapse show' id='grammar'>
							<select 
								className='form-control'>
									{Object.keys(this.grammarMap).map((grammar) => (
										<option key={grammar} value={grammar}>
											{this.grammarMap[grammar]}
										</option>
									))}
							</select>
						</div>
					</div>
					
					{/*UPLOAD*/}
					<div className='form-group'>
						<div className='col-lg-10 col-lg-offset-2 text-center'>
							<a href='#upload'>
								<span className='glyphicon glyphicon-chevron-down'></span>
							</a>
						</div>
						<div className={(this.state.error ? ' has-error' : '') 
							+ (this.state.running ?' has-success' : '')}>
							<label className='col-lg-2 control-label'>
								Upload
							</label>
						</div>
						<div className='col-lg-10 collapse' id='upload'>
							<input 
								type='file'
								accept='.txt'/>
						</div>
					</div>

					<hr/>
					
					{/*STATUS*/}
					<div className={'text-center form-group'
						+ (this.state.error ? ' has-error' : '')
						+ (this.state.running ?' has-success' : '')}>
						{/*<label className='col-lg-2 control-label'>
							Status
						</label>*/}
						<div className='col-lg-12'>
							<button
								className='btn btn-primary'
								onClick={(event) => this.setParser(event)}>
								Parse
							</button>
						</div>
						<div id='parsed' className='col-lg-12'></div>
					</div>
				</fieldset>
			</form>
		);
	}

	renderError() {
		return (
			<div className='alert alert-dismissible alert-danger'>
				<button
					type='button'
					className='close'
					onClick={() => this.setState({ error: '' })}>
					&times;
				</button>
				{this.state.error}
			</div>
		);
	}

	setListeners() {
		var select = $('#grammar select');
		var parser = $('#parser');
		
		var selected = 'selected';
		var show = 'show';
		var speed = 'fast';
	
		initSelect();
		initCollapse();

		function initSelect() {
			selection('option:first');
			
			$(select).on('change', function() {
				$(this).find('option[' + selected + ']').removeAttr(selected);
				selection("option[value='" + this.value + "']");
			});
		}		
		
		function selection(option) {
			$(select).find(option).attr(selected, '');
		}
		
		function initCollapse() {
			$(parser).find('a').click(function(e) {
				e.preventDefault();
				collapse(this);
			});
		}
		
		function collapse(link) {
			var id = $(link).attr('href');
			var div = $(parser).find(id);
			var collapsed = !$(div).hasClass(show);
			
			$('.collapse').each(function() {
				hide(this);
				toggleIcon(this, 'up', 'down');
			});
			
			if (collapsed) {
				expand(div);
				toggleIcon(div, 'down', 'up');
			}
		}
		
		function hide(div) {	
			$(div).fadeOut(speed, function() {
			    $(this).removeClass(show);
			});
		}
		
		function expand(div) {
			$(div).fadeIn(speed, function() {
			    $(this).addClass(show);
			});
		}
		
		function toggleIcon(div, prevDir, nextDir) {
			var icon = 'glyphicon-chevron-';
			var span = $(div).parent().find('.' + icon + prevDir);
			$(span).removeClass(icon + prevDir).addClass(icon + nextDir);
		}
	}

	initGrammar() {
		var show = $('.show');
		
		var select = 'select';
		var input = 'input';
	
		this.setGrammar('');
		
		if (this.active(select)) { 
			var grammar = $(show).find(select).find('option:selected').text();
			this.setGrammar(grammar);
		} else
			if (this.active(input)) {
				var file = $(show).find(input).prop('files')[0];
				
				if (file != null) {
					this.props.syndred.read(file, this, this.upload)
					this.setGrammarParsed(location.hash, file.name);
				}
		}
	}
	
	setGrammar(grammar) {
		this.setGrammarParsed(grammar, grammar);
	}
	
	setGrammarParsed(grammar, parsed) {
		this.state.grammar = grammar;
		$('#parsed').html(parsed);
	}
	
	active(formElement) {
		return $('#parser .show').has(formElement).length;
	}
	
	// TODO empty grammar?
	upload(instance, content) {
		instance.props.socket
			.send(`${instance.dest}upload`, { }, content);
	}

}