package syndred.entities;

import java.io.Serializable;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonPropertyOrder;

@JsonInclude(JsonInclude.Include.NON_NULL)
@JsonPropertyOrder({})
public class Data implements Serializable {

	private final static long serialVersionUID = 173633736370913411L;

}
