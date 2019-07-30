import React from 'react';
import ReactDOM from 'react-dom';
import SockJS from 'sockjs-client';
import Stomp from '@stomp/stompjs';

import Editor from './editor';
import Parser from './parser';
import Generator from './generator';

class Syndred extends React.Component {

	constructor(props) {
		super(props);

		this.state = {
			ready: false,
			// TODO: AMQP?
			socket: Stomp.over(new SockJS('/websocket'))
		};
		this.state.socket.debug = null;
		// TODO
		this.state.socket.maxWebSocketFrameSize = /*126*/10000;
	}

	componentDidMount() {
		window.setTimeout(() => this.componentDidMount(), 2500);

		if (!this.state.ready)
			this.state.socket.connect({ instance: location.hash },
				() => this.setState({ ready: true })/* TODO,
				() => this.setState({ ready: false })*/);
	}

	componentWillMount() {
		if (!location.hash)
			location.hash = Math.random().toString(36).substring(2);

		window.dest = '/syndred/' + location.hash;
		window.onhashchange = () => window.location.reload();
	}

	render() {
		return this.state.ready ? (
			<div className='row'>
				<div className='col-md-5'>
					<Parser
						run={() => this.editor.parse()}
						socket={this.state.socket}
						
						syndred={this}
					/>
					<Generator
						tree={() => this.editor.tree()}
						
						socket={this.state.socket}
						syndred={this}
					/>
				</div>
				<div className='col-md-7'>
					<Editor
						ref={(ref) => this.editor = ref}
						socket={this.state.socket}
						
						syndred={this}
					/>
				</div>
			</div>
		) : (
			<div className='row'>
				<div className='col-md-12 text-center'>
					<img className='spinner' src='/spinner.svg' />
					<h1><strong>Connecting</strong></h1>
				</div>
			</div>
		);
	}
	
	read(file, instance, callback) {
		let reader = new FileReader();
		reader.onloadend = (e) => {
			callback(instance, e.target.result);
		};
		reader.readAsText(file);
	}

}

ReactDOM.render(<Syndred />, document.getElementById('syndred'));
