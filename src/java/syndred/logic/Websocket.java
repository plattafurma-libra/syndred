package syndred.logic;

import org.springframework.messaging.simp.config.ChannelRegistration;
import org.springframework.messaging.simp.config.MessageBrokerRegistry;
import org.springframework.stereotype.Component;
import org.springframework.web.socket.config.annotation.AbstractWebSocketMessageBrokerConfigurer;
import org.springframework.web.socket.config.annotation.EnableWebSocketMessageBroker;
import org.springframework.web.socket.config.annotation.StompEndpointRegistry;
import org.springframework.web.socket.config.annotation.WebSocketTransportRegistration;

@Component
@EnableWebSocketMessageBroker
public class Websocket extends AbstractWebSocketMessageBrokerConfigurer {

	private Interceptor interceptor = new Interceptor();

	@Override
	public void registerStompEndpoints(StompEndpointRegistry registry) {
		/*SockJsServiceRegistration reg =*/ registry.addEndpoint("/websocket").withSockJS();
//		reg.setStreamBytesLimit(200 * 1024 * 1024);
	}

	@Override
	public void configureMessageBroker(MessageBrokerRegistry registry) {
		registry.setApplicationDestinationPrefixes("/syndred");
	}

	@Override
	public void configureClientInboundChannel(ChannelRegistration registration) {
		registration.setInterceptors(interceptor);
	}

	@Override
	public void configureClientOutboundChannel(ChannelRegistration registration) {
		registration.setInterceptors(interceptor);
	}
	
	// TODO
	@Override
    public void configureWebSocketTransport(WebSocketTransportRegistration registration) {	
		registration.setSendTimeLimit(60 * 1000)
	        .setSendBufferSizeLimit(200 * 1024 * 1024)
	        .setMessageSizeLimit(200 * 1024 * 1024);
    }

}
