import React from 'react';

export default class Generator extends React.Component {
	
	constructor(props) {
		super(props);
	}
	
	/*componentDidMount() {}*/
	
	render() {
		return (
			<form className='form-horizontal well'>
				<fieldset>
					<legend>Generator</legend>
					
					<div className='form-group'>
						<label className='col-lg-2'>
							Download
						</label>
						<div className='col-lg-4'>
							<button
								className='btn btn-block btn-primary'
								onClick={(event) => this.download(event)}>
								Apply
							</button>
						</div>
					</div>
				</fieldset>
			</form>);
	}
	
	download(event) {
		event.preventDefault();
		
		// TODO download JSON
		console.log(this.props.tree());
	}
	
}