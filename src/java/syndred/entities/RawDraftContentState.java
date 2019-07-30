package syndred.entities;

import java.io.Serializable;
import java.util.List;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.annotation.JsonPropertyOrder;

@JsonInclude(JsonInclude.Include.NON_NULL)
@JsonPropertyOrder({ "generator", "parseTree", "entityMap", "blocks" })
public class RawDraftContentState implements Serializable {

	@JsonProperty("generator")
	private String generator;
	
	@JsonProperty("parseTree")
	private String parseTree;

	@JsonProperty("entityMap")
	private EntityMap entityMap;

	@JsonProperty("blocks")
	private List<Block> blocks = null;

	private final static long serialVersionUID = 8697339451658053396L;
	
	@JsonProperty("generator")
	public String isGenerator() {
		return generator;
	}

	@JsonProperty("generator")
	public void setGenerator(String generator) {
		this.generator = generator;
	}

	@JsonProperty("parseTree")
	public String getParseTree() {
		return parseTree;
	}

	@JsonProperty("parseTree")
	public void setParseTree(String parseTree) {
		this.parseTree = parseTree;
	}

	@JsonProperty("entityMap")
	public EntityMap getEntityMap() {
		return entityMap;
	}

	@JsonProperty("entityMap")
	public void setEntityMap(EntityMap entityMap) {
		this.entityMap = entityMap;
	}

	@JsonProperty("blocks")
	public List<Block> getBlocks() {
		return blocks;
	}

	@JsonProperty("blocks")
	public void setBlocks(List<Block> blocks) {
		this.blocks = blocks;
	}

}
