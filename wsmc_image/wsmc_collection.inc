<?php



/**
 * Callback for creating image resources.
 *
 * @param object $node
 * @return object
 */
function _wsmc_collection_create( $node) {
  	global $user;

		$node = _services_arg_value($node, 'node');
		
		
		$newNode = (object) array('type' => 'collection');
		node_object_prepare($newNode);
		$newNode->type = 'collection';
		$newNode->title = isset($node['name']) ? $node['name'] : 'Collection';
		$newNode->uid = 0;
		$newNode->comment = 0;
		$newNode->promote = 0;
		$newNode->moderate = 0;
		$newNode->sticky = 0;
		$newNode->language = 'und';
		$newNode->field_img_path['und'][0] = (array)$file;
		
		// Save the node.
	    node_submit($newNode);  
	    node_save($newNode); 
		
		$output = $newNode;
		
	return (object)$output;
}








