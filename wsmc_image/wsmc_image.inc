<?php


/*
 *          MANTAINED FUNCTIONS
 * 
_wsmc_image_create ok
_wsmc_image_update ok
_wsmc_image_retrieve ok
_wsmc_image_delete ok (bisogna cancellare in cascata le annotazioni attaccate?? parliamone)
_wsmc_image_index ok
_wsmc_image_index_light ok

_wsmc_image_access non utilizzata?? ok

save_annotation ok
save_segmentation ok
save_mediasegment ok

_wsmc_task_create ok
_wsmc_task_update  impostare il trigger per impostare come chiusi tutti i utask associati
_wsmc_task_index ok
_wsmc_task_retrieve ok

_wsmc_utask_create c'è un node_load, to fix
_wsmc_utask_update c'è un node_load, to fix
_wsmc_utask_retrieve c'è un node_load, to fix

_wsmc_content_index ok
_wsmc_content_retrieve c'è un node_load ok

_wsmc_textAnnotation_index richiama la retrive che ha NL
_wsmc_textAnnotation_retrieve c'è NL, to fix

_wsmc_polylinedescription_index richiama la retrive che ha NL
_wsmc_polylinedescription_retrieve c'è NL, to fix

_wsmc_action_index richiama la retrive che ha NL 
_wsmc_action_retrieve c'è NL, to fix
_wsmc_action_create ok
_wsmc_action_update ok

_wsmc_session_index ok
_wsmc_session_retrieve 
_wsmc_session_create ok
_wsmc_session_update  update da fare tramite SQL UPDATE

_wsmc_cubrikuser_retrieve secondo me non funziona la query... chi lo chiama?? ignoriamo per ora
_wsmc_cubrikuser_index secondo me non funziona la query... chi lo chiama?? ignoriamo per ora


_wsmc_collection_create ok
_wsmc_collection_retrieve_images ok
_wsmc_collection_retrieve_tasks ok
_wsmc_collection_retrieve ok
_wsmc_collection_index ok
_wsmc_collection_delete ok
 * 
 * */













/********************************************************************************
 * 					IMAGE
 * ******************************************************************************
 */
// wsmc_image.inc






/**
 * Callback for creating image resources.
 *
 * @param int $image_id
 * @param string $operation
 * @param object $node
 * @return object
 */
function _wsmc_image_create($image_id, $operation, $node) {
  global $user;
	//SE SI STA INSERENDO UN CONTENUTO RELATIVO A UNA OPERAZIONE
	if(isset($image_id) && !empty($image_id)){
		$output = _wsmc_image_update($image_id, $operation, $node);
	}else{
		//ALTRIMENTI SI STA INSERENDO UN'IMMAGINE NUOVA
		$node = _services_arg_value($node, 'node');
		$path = isset($node['url']) ? $node['url'] : '';
		$base64image = isset($node['base64image']) ? $node['base64image'] : '';
		if($base64image != ''){
			$base64image = str_replace(' ','+',$base64image);
			$image = base64_decode($base64image);
			$baseName = isset($node['basename']) ? $node['basename'] : 'image.jpg';
		}else{
			$image = file_get_contents($path); // this is a PHP function to get a string representation of the image file
			$baseName = basename($path);	
		}
		//$size = getimagesize($url);
		
		if($image !== FALSE){
			
			$destination = 'public://images/'.$baseName; // a stream wrapper path where you want this image to reside on your file system including the desired filename
			
			$file = file_save_data($image, $destination, FILE_EXISTS_RENAME);
			if (is_object($file)) { // if you get back a Drupal $file object, everything went as expected so make the status permenant
			   $file->status = 1;
			   $file = file_save($file);
			}
			$newNode = (object) array('type' => 'image');
			node_object_prepare($newNode);
		  	$newNode->type = 'image';
		  	$newNode->title = isset($node['title']) ? $node['title'] : 'Image';
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
		}else{
			$output['error'] = t(print_error_string('not_valid','File'));
		}
	}
	return (object)$output;
}

/**
 * Callback for updating image resources.
 *
 * @param int $id
 * @param object $data
 * @return object
 */
function _wsmc_image_update($image_id, $operation, $node) {
	$node = _services_arg_value($node, 'node');
  	global $user;
	switch ($operation){
		case 'segmentation':
			$output = save_segmentation($image_id, $node);
			break;
		case 'textAnnotation':
			
			$output = save_annotation($image_id, $node);
			return $output;
			break;
			
		case 'polylineDescription':
			
			$output = save_mediasegment($image_id, $node);
			break;
		default:
			$output['error'] = print_error_string('not_found','Operation',$operation);
	}
	return (object)$output;
} 

/**
 * Callback for retrieving image resources.
 *
 * @param int $id
 * @return object
 */
function _wsmc_image_retrieve($id) {
	global $base_root;
	global $base_path;
	
	// array per inserire i dati finali da stampare
	$final_result = array();	
	
	if($id=='all'):	$final_result = _wsmc_image_index();				
	else:
		if($id=='light'):	$final_result = _wsmc_image_index_light(null, null, 'id');		
		else:

			$img = array();
			$records = db_query("SELECT id, uri, width, height, filesize, filemime, filename FROM {imginfo} where id= :id", array(':id' => $id));
			foreach ($records as $record) {
  
				$img = array();
				$img['id'] = $record->id;		
				$img['mediaLocator'] = "sites/default/files/images/" . substr($record->uri, 16);
				$img['width'] = $record->width;
				$img['height'] = $record->height;
				$img['filesize'] = $record->filesize;
				$img['filemime'] = $record->filemime;
				$img['filename'] = $record->filename;
				$content = array();

				$content['availableTags'] = image_retrieve_available_tags($img['id']);
				$content['tagging'] = image_retrieve_tags($img['id']);
				$content['segmentation'] = image_retrieve_annotations($img['id']);
	
				$img['descriptions'] = $content;
    			
  				return $img;
  			} 

  			$final_result = t(print_error_string('not_valid','image id',$id));	
		endif;
	endif;	
  return $final_result;
}

/**
 * Callback for deleting image resources.
 *
 * @param int $id
 * @return object
 */
function _wsmc_image_delete($id) {
	node_delete($id);
  	return 'delete';
}

function _wsmc_image_index() {
  $toReturn = array();
  /*db_query("DROP VIEW IF EXISTS annotations;
  			DROP VIEW IF EXISTS textannotation;
			DROP VIEW IF EXISTS segmentation;
			DROP VIEW IF EXISTS tags;
			DROP VIEW IF EXISTS tmpimg;
			DROP VIEW IF EXISTS imginfo;
			CREATE VIEW annotations AS (SELECT CD.entity_id AS contentid, CD.field_content_image_nid AS imageid FROM field_data_field_content_image AS CD);
			CREATE VIEW textannotation AS (SELECT imageid, entity_id FROM field_data_field_content_name as C JOIN annotations as A WHERE C.field_content_name_value = 'tagging' AND C.entity_id = A.contentid);
			CREATE VIEW segmentation AS (SELECT imageid, entity_id FROM field_data_field_content_name as C JOIN annotations as A WHERE C.field_content_name_value = 'segmentation' AND C.entity_id = A.contentid);
			CREATE VIEW tmpimg AS (SELECT id,uri,filesize,filemime,filename FROM file_managed JOIN file_usage WHERE file_managed.fid = file_usage.fid);
			CREATE VIEW imginfo AS (SELECT id,uri, field_img_path_width AS width, field_img_path_height AS height,filesize,filemime,filename FROM tmpimg JOIN field_data_field_img_path WHERE tmpimg.id = field_data_field_img_path.entity_id);");*/
  $result = db_query("SELECT id, uri, width, height, filesize, filemime, filename FROM {imginfo}");

  foreach ($result as $record) {
	$img = array();
	$img['id'] = $record->id;		
	$img['mediaLocator'] = "sites/default/files/images/" . substr($record->uri, 16);
	$img['width'] = $record->width;
	$img['height'] = $record->height;
	$img['filesize'] = $record->filesize;
	$img['filemime'] = $record->filemime;
	$img['filename'] = $record->filename;
	$content = array();

	$content['availableTags'] = image_retrieve_available_tags($img['id']);
	$content['tagging'] = image_retrieve_tags($img['id']);
	$content['segmentation'] = image_retrieve_annotations($img['id']);
	
	$img['descriptions'] = $content;
    array_push($toReturn, $img);

  }
  return $toReturn;
}



function _wsmc_image_index_light($collection, $limit, $select) {
	$toReturn = array();

	
	if(isset($select) && $select=='id'){
		if(isset($collection)){
			//retrieve the list of image id from the collection table
  			if(isset($limit)){
  				$query="SELECT field_collection_image_nid as id FROM {field_data_field_collection_image} ORDER BY RAND() LIMIT {$limit}";  		
  			} else {
				$query="SELECT field_collection_image_nid as id FROM {field_data_field_collection_image} ORDER BY RAND()";  		
  			}	
  		} else {
  			// nocollection selected, download all the images
  			if(isset($limit)){
  				$query="SELECT nid as id FROM {node} WHERE type='image' ORDER BY RAND() LIMIT {$limit}";  		
  			} else {
				$query="SELECT nid as id FROM {node} WHERE type='image' ORDER BY RAND()";  		
  			}	
  		}
  		$result = db_query($query);
		foreach ($result as $record) {
				$img = array();
				$img['id'] = $record->id;			
  				array_push($toReturn, $img);
  		}
  		return $toReturn;
	} else {
		//all the attributes
		if(isset($collection)){
			//retrieve the list of image id from the collection table
  			if(isset($limit)){
  				$query="SELECT id, uri, width, height, filesize, filemime, filename FROM {field_data_field_collection_image} AS C JOIN {imginfo} AS I WHERE C.field_collection_image_nid=I.id	AND C.entity_id='{$collection}' ORDER BY RAND() LIMIT {$limit}";  		
  			} else {
				$query="SELECT id, uri, width, height, filesize, filemime, filename FROM {field_data_field_collection_image} AS C JOIN {imginfo} AS I WHERE C.field_collection_image_nid=I.id	AND C.entity_id='{$collection}' ORDER BY RAND()";  		
  			}	
  		} else {
  			// nocollection selected, download all the images
  			if(isset($limit)){
  				$query="SELECT id, uri, width, height, filesize, filemime, filename FROM {imginfo} ORDER BY RAND() LIMIT {$limit}";  		
  			} else {
				$query="SELECT id, uri, width, height, filesize, filemime, filename FROM {imginfo}";  		
  			}
  			
  		}
		$result = db_query($query);

		foreach ($result as $record) {
			$img = array();
			$img['id'] = $record->id;		
			$img['mediaLocator'] = "sites/default/files/images/" . substr($record->uri, 16);
			$img['width'] = $record->width;
			$img['height'] = $record->height;
			$img['filesize'] = $record->filesize;
			$img['filemime'] = $record->filemime;
			$img['filename'] = $record->filename;
			$content = array();

			$content['availableTags'] = image_retrieve_available_tags($img['id']);
			$content['tagging'] = image_retrieve_tags($img['id']);
			$content['segmentation'] = image_retrieve_annotations($img['id']);
	
			$img['descriptions'] = $content;
    
    		array_push($toReturn, $img);
  		}
	}
  return $toReturn;
}


/**
 * Access callback for the image resource.
 *
 * @param string $op
 *  The operation that's going to be performed.
 * @param array $args
 *  The arguments that will be passed to the callback.
 * @return bool
 *  Whether access is given or not.
 */

function _wsmc_image_access($op, $args) {
  global $user;
  $access = FALSE;

  switch ($op) {
    case 'view':
      $image = wsmc_image_get_image($args[0]);
      $access = user_access('image resource view any image');
      $access = $access || $image->uid == $user->uid && user_access('image resource view own images');
      break;
    case 'update':
      $image = wsmc_image_get_image($args[0]->id);
      $access = user_access('image resource edit any image');
      $access = $access || $image->uid == $user->uid && user_access('image resource edit own images');
      break;
    case 'delete':
      $image = wsmc_image_get_image($args[0]);
      $access = user_access('image resource delete any image');
      $access = $access || $image->uid == $user->uid && user_access('image resource delete own images');
      break;
  }
  $access = TRUE;
  
  return $access;
}


/********************************************************************************
 * 					END IMAGE
 * *****************************************************************************/

 

function save_annotation($image_id = NULL, $node)
{
    
	$app_details = get_consumer_by_key($node['oauth_consumer_key']);
	
    $app_id = $app_details['id'];
	// il tag puo essere passato come id o come valore
	$ta_val = $node['ta_val'];
	$ta_id = $node['ta_id'];
	
	
	$ta_name = $node['ta_name'];
	
	// estrago criteria, language e user_id
	$criteria = $node['criteria'];
	$lang_code = $node['language']; 
	$user_id = $node['user_id'];
	$content_type = $node['content_type'];

	$session_id = $node['session_id'];
	
	$action_id = $node['action_id'];
	$action_exist = false;
	
	if(!empty($action_id)):
		 // verifico se e veramente una action
	    if(!verify_node_type($action_id,'action')):
	        $output['error'] = t(print_error_string('not_valid','session id',$action_id));
	        return $output;
	    endif;		 
		 $action_exist = true;
	else:
		 $action_exist = false;
	endif;
	//  verify if fields have values. if at least one required field does not have a value we throw an error

    $array = array('ta', 'ta_name', 'content_type', 'language', 'user_id', 'session_id','image_id','oauth_consumer_key');  //  required fields names
    $fields_to_ceck = implode(",", $array);
	
    $fields_values = array(         // required fields values
            'ta_val' => $ta_val,
            'ta_id' => $ta_id,
            'ta_name' => $ta_name,
            'content_type' => $content_type,
            'language' => $lang_code,
            'user_id' => $user_id,
            'session_id' => $session_id,
            'image_id'=>$image_id,
            'oauth_consumer_key' => $node['oauth_consumer_key']);
	
    $verified = require_fields($fields_to_ceck,$fields_values);

    if($verified):  // if at least one field value is missing we throw an error
    
        $output['error'] = t(print_error_string('required',join(', ', $verified))); 
        return $output;
    endif;

    
    // get a cubrik_user_id
    $cubrik_user_id = process_cubrik_user($app_id,$user_id);
    	
	//verify the session id
/*    if(!verify_node_type($session_id,'session')):
        $output['error'] = t(print_error_string('not_valid','session id',$session_id));
        return $output;
    endif;

*/
    
	$control = session_verify($session_id);

	if($control!=1):
		$output['error'] = $control;
		return $output;
	endif; 
		

	//verify the user id
 /*   if(!verify_node_type($user_id,'user')):
        $output['error'] = t(print_error_string('not_valid','user id',$user_id));
        return $output;
    endif;
*/
    //verify language value
    $language_id = verify_language($lang_code);
	

    if($language_id==0): 
        $output['error'] = t(print_error_string('not_valid','language code',$lang_code));
        return $output;
    endif;

    //verify if the image is an image
/*   
    if(!verify_node_type($image_id,'image')):

        $output['error'] = t(print_error_string('not_valid','image id',$image_id));
        return $output;
    endif;	
*/

	// control the ta_id
	
	// if we pass the ta_val
	if(empty($ta_id)):	
		// find if this kind of the ta value is presented already
		$ta_id = get_annotation_id($ta_val,$ta_name,$language_id);
		// if the value was not found we insert a new annotation
    	if($ta_id==0) $ta_id = insert_new_annotation($ta_val,$language_id,$ta_name);
    
	endif;
	
/*
	// verify the ta_id
	if(!empty($ta_id)):
        if(!verify_node_type($ta_id,'textannotation')):
            $output['error'] = t(print_error_string('not_valid','ta id',$ta_id));
            return $output;
        endif;	
    endif;
*/

	
	//add new tag content to contents
    $content_id = add_new_content($content_type,$image_id,$ta_id);

    //$content = node_load($content_id);
    //$content_type = $content->field_content_name['und'][0]['value'];

    // if all verification went well, we can finaly create the action
    //return $action_exist;
    if($action_exist):
		$action_node = node_load($action_id);
		
		foreach($action_node->field_action_annotation_id['und'] as $key => $action_ann)
			{

				if($action_ann['nid'] == $ta_id):
					node_delete($content_id);
					return $action_node;
				endif;
			}
		
		$action_node->field_action_annotation_id['und'][]['nid'] = $ta_id;
		
		node_save($action_node);
		// associo alla action esistente l'annotation
	else:
		// altrimente creo una nuova action
    $action = create_action($session_id,$cubrik_user_id,'annotation',NULL,$image_id,$content_id,$ta_id,NULL);
	endif;

	
	$tag_exists=image_contains_tags($image_id, $ta_val, $lang_code);
	if($tag_exists==0){
       save_annotation_availableTags($image_id, $session_id, $language_id, $cubrik_user_id, $ta_val, $ta_name );
	}
	

	return $action;

}




/********************************************************************************
 * 					SEGMENTATION
 * *****************************************************************************/




/**
 * FUNZIONE CHE SALVA I DATI DI UN'OPERAZIONE DI SEGMENTATION
 * 
 * @param $image_id int: id dell'immagine a cui associare le coordinate (il tracciato)
 * @param $nodo array: include dati come label, coordinate lang_code.
 * @return $node del contenuto creato o errore nel caso non venga creato.
 */
 
function save_segmentation($image_id = NULL, $node){

//  field settings

    $app_details = get_consumer_by_key($node['oauth_consumer_key']);
    $app_id = $app_details['id'];
	// il tag puo essere passato come id o come valore
	$ta_val = $node['ta_val'];
	$ta_id = $node['ta_id'];
	$ta_name = $node['ta_name'];	
	// estrago criteria, language e user_id
	$criteria = $node['criteria'];
	$lang_code = $node['language']; 
	$user_id = $node['user_id'];
	$content_type = $node['content_type'];
	$session_id = $node['session_id'];

    $polyline_r = $node['polyline_r'];
    $polyline_h = $node['polyline_h'];

//  verify if fields have values. if at least one required field does not have a value we throw an error

    $array = array('ta', 'ta_name', 'content_type', 'language', 'user_id', 'session_id','image_id','polyline','oauth_consumer_key');  //  required fields names
    $fields_to_ceck = implode(",", $array);

    $fields_values = array(         // required fields values
            'ta_val' => $ta_val,
            'ta_id' => $ta_id,
            'ta_name' => $ta_name,
            'content_type' => $content_type,
            'language' => $lang_code,
            'user_id' => $user_id,
            'session_id' => $session_id,
            'image_id'=>$image_id,
			'polyline_r' => $polyline_r,
			'polyline_h' => $polyline_h,
            'oauth_consumer_key' => $node['oauth_consumer_key']);

    $verified = require_fields($fields_to_ceck,$fields_values);

    if($verified):  // if at least one field value is missing we throw an error
    
        $output['error'] = t(print_error_string('required',join(', ', $verified))); 
        return $output;
    endif;
	
    $cubrik_user_id = process_cubrik_user($app_id,$user_id);

    // verify if session is open
	$control = session_verify($session_id);

	
	if($control!=1):
		$output['error'] = $control;
		return $output;
	endif; 

    //verify language value
    $language_id = verify_language($lang_code);


    //if($language_id==0): 
    //    $output['error'] = t(print_error_string('not_valid','language code',$lang_code));
    //    return $output;
    //endif;

    //verify if the image is an image
     
    //if(!verify_node_type($image_id,'image')):
    //    $output['error'] = t(print_error_string('not_valid','image id',$image_id));
    //    return $output;
    //endif;


	// control the ta_id
	
	// if we pass the ta_val
	//if(empty($ta_id)):	
		// find if this kind of the ta value is presented already
		//$ta_id = get_annotation_id($ta_val,$ta_name,$language_id);
		// if the value was not found we insert a new annotation
    	//if($ta_id==0) $ta_id = insert_new_annotation($ta_val,$language_id,$ta_name);
	//endif;
	


	// verify the ta_id
	if(!empty($ta_id)):
        if(!verify_node_type($ta_id,'textannotation')):
            $output['error'] = t(print_error_string('not_valid','ta id',$ta_id));
            return $output;
        endif;	
    endif;

    $points = array();
    
    if(isset($polyline_r) && $polyline_r!==NULL && isset($polyline_h) &&$polyline_r!==NULL): 
        $point_r_id = insert_point($polyline_r,'result');
        $point_h_id = insert_point($polyline_h,'history');
        
        $r_id['nid']=$point_r_id;
        $h_id['nid']=$point_h_id;
        
        $points[] = $r_id;
        $points[] = $h_id;
        
        $content_id = add_new_content($content_type,$image_id,$ta_id,$points);
    else:
        if(!isset($polyline_h)|| $polyline_h==NULL): 
            $point_id = insert_point($polyline_r,'result');
            $points[0]['nid']=$point_id;
            $content_id = add_new_content($content_type,$image_id,$ta_id,$points);
        endif;
        if(!isset($polyline_r)|| $polyline_r==NULL): 
            $point_id = insert_point($polyline_h,'history');
            $points[0]['nid']=$point_id;
            $content_id = add_new_content($content_type,$image_id,$ta_id,$points);
        endif;       
    endif;
 


    // if all verification went well, we can finaly create the action
    //$content = node_load($content_id);
    //$content_type = $content->field_content_name['und'][0]['value'];
    

	// if all verification went well, we can finaly create the action
	
	$action = create_action($session_id,$cubrik_user_id,'segmentation',NULL,$image_id,$content_id,$ta_id,$points);

	return $action;

}
 
 
/********************************************************************************
 * 					END SEGMENTATION
 * *****************************************************************************/ 

 
/********************************************************************************
 * 					START MEDIASEGMENT
 * *****************************************************************************/ 



/**
 * FUNZIONE CHE SALVA I DATI DI UN'OPERAZIONE DI SEGMENTATION
 * 
 * @param $image_id int: id dell'immagine a cui associare le coordinate (il tracciato)
 * @param $nodo array: include dati come label, coordinate lang_code.
 * @return $node del contenuto creato o errore nel caso non venga creato.
 */
 
function save_mediasegment($image_id = NULL, $node){

    $app_details = get_consumer_by_key($node['oauth_consumer_key']);
    $app_id = $app_details['id'];

	// estrago criteria, language e user_id
	$oauth_consumer_key = $node['oauth_consumer_key'];
	$criteria = $node['criteria'];
	$user_id = $node['user_id'];
	$content_type = $node['content_type'];
	$session_id = $node['session_id'];

    $polyline_r = $node['polyline_r'];
    $polyline_h = $node['polyline_h'];

	// verify if fields have values. if at least one required field does not have a value we throw an error

    $array = array('content_type', 'user_id', 'session_id','image_id','polyline','oauth_consumer_key');  //  required fields names
    $fields_to_ceck = implode(",", $array);

    $fields_values = array(         // required fields values
            'content_type' => $content_type,
            'user_id' => $user_id,
            'session_id' => $session_id,
            'image_id'=>$image_id,
			'polyline_r' => $polyline_r,
			'polyline_h' => $polyline_h,
            'oauth_consumer_key' => $oauth_consumer_key);

    $verified = require_fields($fields_to_ceck,$fields_values);

    if($verified):  // if at least one field value is missing we throw an error
    
        $output['error'] = t(print_error_string('required',join(', ', $verified))); 
        return $output;
    endif;
	
    // get a cubrik_user_id
    $cubrik_user_id = process_cubrik_user($app_id,$user_id);


    // verify if session is open
	$control = session_verify($session_id);

	if($control!=1):
		$output['error'] = $control;
		return $output;
	endif; 


    //verify if the image is an image
     
    //if(!verify_node_type($image_id,'image')):
    //    $output['error'] = t(print_error_string('not_valid','image id',$image_id));
    //    return $output;
    //endif;

    $points = array();
    
    if(isset($polyline_r) && $polyline_r!==NULL && isset($polyline_h) &&$polyline_r!==NULL): 
        $point_r_id = insert_point($polyline_r,'result');
        $point_h_id = insert_point($polyline_h,'history');
        
        $r_id['nid']=$point_r_id;
        $h_id['nid']=$point_h_id;
        
        $points[] = $r_id;
        $points[] = $h_id;
        
        $content_id = add_new_content($content_type,$image_id,NULL,$points);
    else:
        if(!isset($polyline_h)|| $polyline_h==NULL): 
            $point_id = insert_point($polyline_r,'result');
            $points[0]['nid']=$point_id;
            $content_id = add_new_content($content_type,$image_id,NULL,$points);
        endif;
        if(!isset($polyline_r)|| $polyline_r==NULL): 
            $point_id = insert_point($polyline_h,'history');
            $points[0]['nid']=$point_id;
            $content_id = add_new_content($content_type,$image_id,NULL,$points);
        endif;       
    endif;
 
    

	// if all verification went well, we can finaly create the action
	
	$action = create_action($session_id,$cubrik_user_id,'mediasegment',NULL,$image_id,$content_id,NULL,$points,NULL);

	return $action;

}


/********************************************************************************
 * 					END MEDIASEGMENT
 * *****************************************************************************/ 

 
 

/********************************************************************************
 * 					TASK
 * *****************************************************************************/ 

function _wsmc_task_create($data){
  	$errore = FALSE;
  	//Verifico che i campi obbligatori vengano inviati
  	if(!empty($data['taskType']) AND !empty($data['image'])){
    	$id_images = explode(',', $data['image']);
    
    	//Verifico se gli id sono effettivamente di immagini
    	foreach($id_images AS $id){
    		if(is_numeric($id)){
        		$exists = image_exist($id);
        		
        		if(! $exists){
          			//Imposto l'errore
          			$errore = TRUE;
          			$erroreType = 2;
        		}
        		//Nel frattempo preparo i dati da salvare
        		$other_fields['field_task_image']['und'][]['nid'] = $id;
      		}else{
        		$output['error'][] = t(print_error_string('not_valid','image'));
        		return $output;
      		}
    	}
    	//Se non ci sono errori creo il nodo
    	if(!$errore){
      		$node_type = 'task';
      		$title = $data['taskType'];
      		if(!empty($data['aggregationStrategy'])) $other_fields['field_aggregation_strategy']['und'][0]['value'] = $data['aggregationStrategy'];
      		$other_fields['field_task_status']['und'][0]['value'] = 0;
			$other_fields['field_task_type']['und'][0]['value'] = $data['taskType'];
      		$new_node = create_new_node($node_type, $title, $other_fields);
      		node_save($new_node);
      		$output = $new_node;
    	}
  	} else {
    	//Imposto l'errore
    	$errore = TRUE;
    	$erroreType = 1;
  	}
  	//GESTISCO GLI ERRORI
  	if($errore){
    	switch ($erroreType) {
      	case 1:
        	//Nel caso in cui non siano inviati i campi richiesti invio un messaggio di errore
        	if(empty($data['taskType'])) $output['error'][] = t(print_error_string('required','taskType'));
        	if(empty($data['image'])) $output['error'][] = t(print_error_string('required','image'));
        	break;
      	case 2:
        	//errore nel caso l'id inviato non sia di un'immagine
        	$output['error'][] = t(print_error_string('not_valid','image'));
        	break;
      	default:
        	break;
    	}
 	}
	
  return $output;
}

function _wsmc_task_update($id_task, $close){
  	
  	//Verifico l'id del task
  	if(is_numeric($id_task)){

  		$result=db_query('SELECT field_task_status_value AS status , entity_id AS task_id
				FROM {field_data_field_task_status} WHERE entity_id= :id', array(':id' => $id_task));
    	foreach($result AS $key){
    		$task=$key;
    	}

    	if(! isset($task)){
      		$output['error'] = t(print_error_string('not_valid','id task'));
      		return $output;
    	}else{
      		//Se il task è già chiuso imposto l'errore
      		if($task->status == 0)  {
      			$output['error'] = t('Task already closed');
      			return $output;
      		};
    	}
  	}else{
    	$output['error'] = t(print_error_string('not_valid','id task'));
    	return $output;
  	}

 	//Verifico che la stringa passata sia close
  	if($close != 'close') {
  		$output['error'] = t(print_error_string('not_valid','close'));
  		return $output;
  	}
  	
  	$res=db_query('UPDATE {field_data_field_task_status} SET field_task_status_value=0 WHERE entity_id= :id', array(':id' => $id_task));
   	$output[] = t('Task properly closed');
    
  	return $output;
}


function _wsmc_task_index($collection, $limit, $open){
	$output=array();

	if(isset($collection)){
		$query="SELECT T.nid As id, T.title as title, I. field_task_status_value AS status, TY.field_task_type_value AS type
			FROM {field_data_field_collection_task} AS C 
			JOIN {node} AS T
			JOIN {field_data_field_task_type} AS TY
			JOIN {field_data_field_task_status} AS I
			ON T.nid=C.field_collection_task_nid AND T.nid=I.entity_id AND T.nid = TY.entity_id
			WHERE T.type = 'task'AND T.nid=C. field_collection_task_nid AND C.entity_id='{$collection}'";
	} else {
		$query="SELECT T.nid As id, T.title as title, I. field_task_status_value AS status, TY.field_task_type_value AS type FROM {node} AS T
			JOIN {field_data_field_task_status} AS I
			JOIN {field_data_field_task_type} AS TY
			ON  T.nid=I.entity_id AND T.nid = TY.entity_id
 			WHERE T.type = 'task'";
  		
	}
	if(isset($open)){
		$query=$query."AND I.field_task_status_value=1 ";
  	}
  	$query=$query." ORDER BY created";  
  	if(isset($limit)){
		$query=$query." LIMIT {$limit}";
  	}
  
  	$result = db_query($query);
  	// Result is returned as a iterable object that returns a stdClass object on each iteration

	foreach ($result as $record) {

    	$task_values['id'] = $record->id;
    	$task_values['taskType'] = $record->type;
    	$task_values['status'] = $record->status;

    	$task_values['image']= task_retrieve_images($record->id);
    
    	$utask_id = retrieveUTaskByTaskId($record->id);
		$task_values['utasks']= $utask_id;
    
		$output['task'][] = $task_values;
		unset($task_values);
		
  	}
  	return   $output;
}

function _wsmc_task_retrieve($id_task){

  	//Verifico l'id del task
  	if(is_numeric($id_task)){
  		$result = db_query('SELECT T.nid AS id, T.title AS title, I.field_task_status_value AS status, TY.field_task_type_value AS type FROM {node} AS T
			JOIN {field_data_field_task_status} AS I
			JOIN {field_data_field_task_type} AS TY
			ON  T.nid=I.entity_id AND T.nid=TY.entity_id
 			WHERE T.nid = :id', array(':id' => $id_task));
		foreach ($result as $record){
	    	$task_values['id'] = $record->id;
	    	$task_values['taskType'] = $record->type;
    		$task_values['status'] = $record->status;

	    	$task_values['image']= task_retrieve_images($record->id);
    
    		$utask_id = retrieveUTaskByTaskId($record->id);
			$task_values['utasks']= $utask_id;
    
			//$output['task'][] = $task_values;
	  		//unset($utaskList);
			return $task_values;

		}
  	} else {
    	$output['error'] = t(print_error_string('not_valid','id task'));
  	}
  	return $output;
}


/********************************************************************************************************************
 * 					END TASK
 * *****************************************************************************************************************/ 

/********************************************************************************************************************
 * 					uTASK
 * *****************************************************************************************************************/ 



function _wsmc_utask_create($data){
  $errore = FALSE;
  //Verifico che i campi obbligatori vengano inviati
  if(!empty($data['taskType']) AND !empty($data['task'])){
    //Verifico se l'id sia effettivamente di un task
    if(is_numeric($data['task'])){
      $immagine = node_load($data['task']);
      if($immagine->type != task){
        //Imposto l'errore
        $errore = TRUE;
        $erroreType = 2;
      }
    }else{
      $errore = TRUE;
      $erroreType = 2;
    }
    //Verifico che lo status passato non sia errato (deve essere 'open' o 'close')
    if(!empty($data['status']) && $data['status'] != 'close' && $data['status'] != 'open'){
      $errore = TRUE;
      $erroreType = 3;
    }
    //Verifico che l'order sia un intero
    if(!empty($data['order'])){
      if(!is_numeric($data['order'])){
        $errore = TRUE;
        $erroreType = 4;
      }
    }
    //Verifico che l'id della action passato sia effettivamente di un nodo action
    if(!empty($data['action'])){
      if(is_numeric($data['action'])){
        $action = node_load($data['action']);
        if($action->type != 'action'){
          $errore = TRUE;
          $erroreType = 5;
        }
      }else{
        $errore = TRUE;
        $erroreType = 5;
      }
    }
    //Nel caso in cui non ci siano errori salvo il nodo
    if(!$errore){
      $node_type = 'utask';
      $title = $data['taskType'];
      $other_fields['field_task_status']['und'][0]['value'] = $data['status'] == 'close' ? 0 : 1;
      if(!empty($data['order'])) $other_fields['field_utask_order']['und'][0]['value'] = $data['order'];
      if(!empty($data['action'])) $other_fields['field_utask_action']['und'][0]['nid'] = $data['action'];
      $other_fields['field_utask_task']['und'][0]['nid'] = $data['task'];
	  $other_fields['field_utask_type']['und'][0]['value'] = $data['taskType'];
      $new_node = create_new_node($node_type, $title, $other_fields);
      node_save($new_node);
      $output = $new_node;
    }
  }else{
    //Imposto l'errore
    $errore = TRUE;
    $erroreType = 1;
  }
  //GESTISCO GLI ERRORI
  if($errore){
    switch ($erroreType) {
      case 1:
        //Nel caso in cui non siano inviati i campi richiesti invio un messaggio di errore
        if(empty($data['taskType'])) $output['error'][] = t(print_error_string('required','taskType'));
        if(empty($data['task'])) $output['error'][] = t(print_error_string('required','task'));
        break;
      case 2:
        //errore nel caso l'id inviato non sia di un'immagine
        $output['error'][] = t(print_error_string('not_valid','task'));
        break;
      case 3:
        //errore nel caso in cui lo stato inviato non sia di valore diverso da 0 o 1
        $output['error'][] = t(print_error_string('not_valid','status'));
        break;
      case 4:
        //errore nel caso in cui l'ordine non sia un intero
        $output['error'][] = t(print_error_string('not_valid','order'));
        break;
      case 5:
        //errore nel caso in cui l'id della action sia sbagliata
        $output['error'][] = t(print_error_string('not_valid','action'));
        break;
      default:
        break;
    }
  }
  return $output;
}


function _wsmc_utask_update($id_utask, $id_action, $close){
  $erroreType = 0;
  //Verifico l'id del task
  if(is_numeric($id_utask)){
    $utask = node_load($id_utask);
    if($utask->type != 'utask'){
      $erroreType = 1;
    }else{
      //Se il utask è già chiuso imposto l'errore
      if($utask->field_utask_status['und'][0]['value'] == 0)  $erroreType = 3;
    }
  }else{
    $erroreType = 1;
  }
  //Verifico l'id della action
  if(is_numeric($id_action)){
    $action = node_load($id_action);
    if($action->type != 'action'){
      $erroreType = 4;
    }else{
      //Se la action è già associata a un utask imposto l'errore
      $result = db_query('SELECT COUNT(f.entity_id) AS numaction FROM {field_revision_field_utask_action} f WHERE f.field_utask_action_nid = :fid', array(':fid' => $id_action));
      foreach ($result as $record) {
        if($record->numaction > 0){
          $erroreType = 5;
        }
      }
    }
  }else{
    $erroreType = 4;
  }
  //Verifico che la stringa passata sia close
  if($close != 'close') $erroreType = 2;
  //verifico che se è stato passato l'id della action
  //Se è stato impostato un errorType, allora imposto l'error a TRUE
  if($erroreType != 0) $errore = TRUE;
  if($errore){
    switch ($erroreType) {
      case 1:
        //Nel caso in cui l'id non sia corretto
        $output['error'] = t(print_error_string('not_valid','id uTask'));
        break;
      case 2:
        //Nel caso in cui $close non sia 'close'
        $output['error'] = t(print_error_string('not_valid','close'));
        break;
      case 3:
        //Nel caso in cui il task sia già chiuso
        $output['error'] = t('Task already closed');
        break;
      case 4:
        //Nel caso in cui l'id della action sia sbagliato
        $output['error'] = t(print_error_string('not_valid','action id'));
        break;
      case 5:
        //Nel caso in cui la action sia già associata a un'altro uTask
        $output['error'] = t('Action already associated to uTask');
        break;
      default:
        break;
    }
  }else{
   //$utask->field_utask_status['und'][0]['value'] = 0;
   //$utask->field_utask_action['und'][0]['nid'] = $id_action;
   //node_save($utask);
   
   $res=db_query('UPDATE {field_data_field_utask_status} SET field_utask_status_value=0 WHERE entity_id= :id', array(':id' => $id_utask));
   
   if(empty($utask->nid)){
     //$output['error'] = t('Error saving, contact service administrator');
   }else{
     $output[] = t('uTask properly closed');
   }
  }
  return $output;
}


function _wsmc_utask_retrieve($id_utask){
  //Verifico l'id del utask
  if(is_numeric($id_utask)){
    $uTask = node_load($id_utask);
    if($uTask->type != 'utask'){
      $output['error'] = t(print_error_string('not_valid','id uTask'));
    }else{
      $output['uTask'] = array(
          'id' => $uTask->nid,
          'utaskType' => $uTask->field_utask_type['und'][0]['value'],
          'status' => $uTask->field_utask_status['und'][0]['value'],
          'order' => $uTask->field_utask_order['und'][0]['value'],
          'action' => $uTask->field_utask_action['und'][0]['nid'],
          'task' => $uTask->field_utask_task['und'][0]['nid'],
      );
    }
  }else{
    $output['error'] = t(print_error_string('not_valid','id uTask'));
  }
  return $output;
}
/******************************************************************************************************************
 * 					END uTASK
 * ***************************************************************************************************************/ 
 
 
 
 /******************************************************************************************************************
 * 					 CONTENT
 * ***************************************************************************************************************/ 
 
 function _wsmc_content_index(){
  	$result = db_query('SELECT n.nid FROM {node} n WHERE n.type = :type', array(':type' => 'content'));
  	foreach ($result as $record) {
    	$output[] = _wsmc_content_retrieve($record->nid);
    }
  	return   $output;
}
 
 function _wsmc_content_retrieve ($id_content)
 {
    $content = node_load($id_content);
    $output['id'] = $id_content;
    $output['name'] = $content->field_content_name['und'][0]['value'];				
    $result = db_query('SELECT f.field_content_valid_value AS valid FROM {field_data_field_content_valid} f WHERE f.entity_id = :contentid', array(':contentid' => $id_content));
	foreach ($result as $record) {
		$output['valid'] = $record->valid;
	}

	// load annotations
	$annotations = $content->field_content_textannotation['und'];
	foreach ($annotations as $key => $annotation)
		{
			
			$output['itemAnnotations'][] = _wsmc_textAnnotation_retrieve($annotation['nid']);
			
		}
			
	// load points
	$points = $content->field_content_point['und'];
	foreach ($points as $key => $point)
		{
			$output['mediaSegment'][] = _wsmc_polylinedescription_retrieve($point['nid']);			
	
		}	
	
	return $output;
 }
 
 function _wsmc_content_update($id_content, $invalidate){
  	
  	//Verifico l'id del content
  	if(is_numeric($id_content)){

  		$result=db_query('SELECT field_content_valid_value AS status , entity_id AS content_id
				FROM {field_data_field_content_valid} WHERE entity_id= :id', array(':id' => $id_content));
    	foreach($result AS $key){
    		$task=$key;
    	}

    	if(! isset($task)){
      		$output['error'] = t(print_error_string('not_valid','id content'));
      		return $output;
    	}else{
      		//Se il content è già not valid imposto l'errore
      		if($task->status == 0)  {
      			$output['error'] = t('Content already invalidated');
      			return $output;
      		};
    	}
  	}else{
    	$output['error'] = t(print_error_string('not_valid','id content'));
    	return $output;
  	}

 	//Verifico che la stringa passata sia invalidate
  	if($invalidate != 'invalidate') {
  		$output['error'] = t(print_error_string('not_valid','invalidate'));
  		return $output;
  	}
  	
  	$res=db_query('UPDATE {field_data_field_content_valid} SET field_content_valid_value=0 WHERE entity_id= :id', array(':id' => $id_content));
   	$output[] = t('Content properly invalidated');
    
  	return $output;
}


 
 /**************************************************************************************************************
 * 					END CONTENT
 * ***********************************************************************************************************/ 

 /**************************************************************************************************************
 * 					 TEXTANNOTATION
 * ***********************************************************************************************************/ 
 

 function _wsmc_textAnnotation_index(){
  	$result = db_query('SELECT n.nid FROM {node} n WHERE n.type = :type', array(':type' => 'textannotation'));
	// Result is returned as a iterable object that returns a stdClass object on each iteration
  	foreach ($result as $record) {
    	$output[] = _wsmc_textAnnotation_retrieve($record->nid);
  	}
  	return   $output;
}
 
 function _wsmc_textAnnotation_retrieve ($id_textAnnotation)
 {
    $textAnnotation = node_load($id_textAnnotation);
    $output['id'] = $id_textAnnotation;
    $output['name'] = $textAnnotation->field_annotation_name['und'][0]['value'];
	$output['language'] = print_language_data($textAnnotation->field_annotation_language['und'][0]['tid']);
	$output['value'] = $textAnnotation->field_annotation_value['und'][0]['value'];    
	
	return $output;
 }


 /************************************************************************************************************************
 * 					END TEXTANNOTATION
 * *********************************************************************************************************************/ 
  
 /************************************************************************************************************************
 * 					 POLYLINEDESCRIPTION
 * *********************************************************************************************************************/ 
 

function _wsmc_polylinedescription_index(){
	$result = db_query('SELECT n.nid FROM {node} n WHERE n.type = :type', array(':type' => 'point'));
	// Result is returned as a iterable object that returns a stdClass object on each iteration
  	foreach ($result as $record) {
    	$output[] = _wsmc_polylinedescription_retrieve($record->nid);
  	}
  	return   $output;
}
 
function _wsmc_polylinedescription_retrieve ($id_polylinedescription) {
    $polylinedescription = node_load($id_polylinedescription);
    $output['id'] = $id_polylinedescription;
    $output['name'] = $polylinedescription->field_point_name['und'][0]['value'];
	$output['polyline'] = $polylinedescription->field_point_value['und'][0]['value'];    
	
	return $output;
 }


 /************************************************************************************************************************
 * 					END POLYLINEDESCRIPTION
 * ************************************************************************************************************************/ 
   
 /************************************************************************************************************************
 * 					 ACTION
 * ************************************************************************************************************************/ 
 

function _wsmc_action_index(){
	$result = db_query('SELECT n.nid FROM {node} n WHERE n.type = :type', array(':type' => 'action'));
	// Result is returned as a iterable object that returns a stdClass object on each iteration
  
	foreach ($result as $record) {
    	$output[] = _wsmc_action_retrieve($record->nid);
  	}
  
  return   $output;
}
 
function _wsmc_action_retrieve ($id_action) {
    $action = node_load($id_action);
    $output['id'] = $id_action;
    if(!$action){
		$output['error'] = 'not found';
    }
    else{
    	$output['startTs'] = $action->field_action_startts_val['und'][0]['value'];
		$output['endTs'] = $action->field_action_endts_val['und'][0]['value'];    
		$output['type'] = $action->field_action_type_val['und'][0]['value'];    
		$output['user'] = print_user_data($action->field_action_user_id['und'][0]['nid']);
		$output['quality'] = _wsmc_quality_retrieve(1);
		$output['contentid'] = $action->field_action_content_id['und'][0]['nid'];
		$output['imageid'] = $action->field_action_image_id['und'][0]['nid'];
	}
	
	return $output;
 }
 
function _wsmc_action_create($dati) {
    $app_details = get_consumer_by_key($dati['oauth_consumer_key']);
    $app_id = $app_details['id'];
    $session_id = $dati['session_id'];
    $action_log = $dati['action_log'];
    $action_type = $dati['action_type'];
    $user_id = $dati['user_id'];
     
    $array = array('action_type', 'user_id', 'session_id','app_id','action_log');  //  required fields names
    $fields_to_ceck = implode(",", $array);

    $fields_values = array(         // required fields values
            'action_type' => $action_type,
            'user_id' => $user_id,
            'session_id' => $session_id,
            'app_id' => $app_id,
            'action_log' => $action_log);

    $verified = require_fields($fields_to_ceck,$fields_values);

    if($verified):  // if at least one field value is missing we throw an error
        $output['error'] = t(print_error_string('required',join(', ', $verified)));
        return $output;
    endif;    
 
   
 
    // verify if session is open
    $control = session_verify($session_id);

    if($control!=1):
        $output['error'] = $control;
        return $output;
    endif;  
     
    //$control = _wsmc_cubrikuser_index($app_id);

    // if application not exists return an error string
    //if(is_string($control)) return $control;
    
    // get a cubrik_user_id
    $cubrik_user_id = process_cubrik_user($app_id,$user_id);
     
    $action = create_action($session_id,$cubrik_user_id,$action_type,$action_log,NULL,NULL,NULL,NULL);
    return $action;
 }
 
 function _wsmc_action_update($action_id, $dati)
 {
    // verifichiamo prima se la action e veramente una action
    if(!verify_node_type($action_id,'action')):
        $output['error'] = t(print_error_string('not_valid','action id',$action_id));
        return $output;
    endif;
     
     
    $app_details = get_consumer_by_key($dati['oauth_consumer_key']);
    $app_id = $app_details['id'];
    $session_id = $dati['session_id'];
    $action_log = $dati['action_log'];
    $action_type = $dati['action_type'];
    $user_id = $dati['user_id'];
     
    $array = array('action_type', 'user_id', 'session_id','app_id','action_log','action_id');  //  required fields names
    $fields_to_ceck = implode(",", $array);

    $fields_values = array(         // required fields values
            'action_type' => $action_type,
            'user_id' => $user_id,
            'session_id' => $session_id,
            'app_id' => $app_id,
            'action_id' => $action_id,
            'action_log' => $action_log);

    $verified = require_fields($fields_to_ceck,$fields_values);

    if($verified):  // if at least one field value is missing we throw an error
    
        $output['error'] = t(print_error_string('required',join(', ', $verified)));

        return $output;
    endif;  
  
 
    // verify if session is open
    $control = session_verify($session_id);

    if($control!=1):
        $output['error'] = $control;
        return $output;
    endif;  
     
    //$control = _wsmc_cubrikuser_index($app_id);

    // if application not exists return an error string
    //if(is_string($control)) return $control;
    
    // get a cubrik_user_id
    $cubrik_user_id = process_cubrik_user($app_id,$user_id);

   
    $action_node = node_load($action_id);
    
    $action_node->field_action_type_val['und'][0]['value'] = $action_type;
    
    $action_node->field_action_log_val['und'][0]['value'] = $action_log;
    
    // add the session id
    $action_node->field_action_session_id['und'][0]['nid'] = $session_id;
    
    // add the user id
    $action_node->field_action_user_id['und'][0]['nid'] = $cubrik_user_id;

    node_save($action_node);
    
    // aggiorno anche la session nel caso se si e aggiunto un nuovo utente
    
    
    // update user list
    update_session_user_list($session_id,$cubrik_user_id);
    
     return $action_node;       

 }
 
 
 
 /*********************************************************************************************************************
 * 					END ACTION
 * ******************************************************************************************************************/ 
    

/*********************************************************************************************************************
 * 					SESSION
 * ******************************************************************************************************************/ 


function _wsmc_session_index(){
	$out_tot=array();
		
	$result = db_query('SELECT N.nid AS ID, ST.field_session_startts_value AS startTs, ET.field_session_endts_value AS endTs
					FROM  {node} AS N
					JOIN  {field_data_field_session_startts} AS ST
					LEFT JOIN {field_data_field_session_endts} AS ET
					ON ET.entity_id=N.nid
					WHERE N.type =  :s
					AND ST.entity_id=N.nid', array(':s' => 'session'));
  		
  
	foreach ($result as $record) {
		$output['id'] = $record->ID;
    	$output['startTs'] = $record->startTs;
		$output['endTs'] = $record->endTs;
		$output['users'] = session_retrieve_users($record->ID);
		$output['actions'] = session_retrieve_actions($record->ID);
 		$out_tot[]=$output;
 		unset($output);
  	}
 	return   $out_tot;
 		
}
 
function _wsmc_session_retrieve ($id_session) {
 	$output=array();

    $records = db_query('SELECT ST.field_session_startts_value AS startTs, ET.field_session_endts_value AS endTs
					FROM  {node} AS N
					JOIN  {field_data_field_session_startts} AS ST
					LEFT JOIN {field_data_field_session_endts} AS ET
					ON ET.entity_id=N.nid
					WHERE N.nid =  :s
					AND ST.entity_id=N.nid', array(':s' => $id_session));

    foreach ($records as $record) {
    	$output['id'] = $id_session;
    	
    	$output['startTs'] = $record->startTs;
		$output['endTs'] = $record->endTs;

		$output['users'] = session_retrieve_users($id_session);
		
		$output['actions'] = session_retrieve_actions($id_session);
		
		return $output;
	}

	$output['error'] = t(print_error_string('not_valid','id task'));
	
	return $output;
}
 


function _wsmc_session_create($dati) {
		
	$app_details = get_consumer_by_key($dati['oauth_consumer_key']);
    $app_id = $app_details['id'];
        
    $array = array('oauth_consumer_key');  //  required fields names
    $fields_to_ceck = implode(",", $array);

    $fields_values = array('oauth_consumer_key' => $dati['oauth_consumer_key']);

    $verified = require_fields($fields_to_ceck,$fields_values);
    
    if($verified):  // if at least one field value is missing we throw an error
        $output['error'] = t(print_error_string('required',join(', ', $verified))); 
        return $output;
    endif;
        
	// creo il nuovo nodo
	
	$node = create_new_node('session', 'Session');

	// valorifico lo stato
	$node->field_session_startts['und'][0]['value'] = time();
			
	$node->field_session_user['und'][0]['nid'] = $user_id;
	// associo l'applicazion
	$node->field_session_application_id['und'][0]['value'] = $app_id;
	global $user;
	$user = user_load(1);
	//node_save_action($node);
    node_save($node);
	
	
			
	return $node->nid;
}



/**
 * Callback for updating session state.
 *
 * @param int $session_id
 * @param object $data
 */
function _wsmc_session_update($session_id, $node) {

	$state = $node['state'];

	
    $array = array('state', 'session_id');  //  required fields names
    $fields_to_ceck = implode(",", $array);

    $fields_values = array(         // required fields values
            'session_id' => $session_id,
            'state' => $state,
	);

    $verified = require_fields($fields_to_ceck,$fields_values);

    if($verified):  // if at least one field value is missing we throw an error
        $output['error'] = t(print_error_string('required',join(', ', $verified))); 
        return $output;
    endif;

    // verify if session is open
    //$control = session_verify($session_id);

    if($control!=1):
        //$output['error'] = $control;
        //return $output;
    endif;  
	
	$session_node = node_load($session_id);
	// if we want to update the session from closed to opened
	if($state):
		$session_node->field_session_endts['und'][0]['value'] = NULL;	
		$result = 'opened';
	endif;

	// if we want to update the session from opened to closed
	if(!$state):
		$session_node->field_session_endts['und'][0]['value'] = time();	
		$result = 'closed';
	endif;

	node_save($session_node);

	return $session_node->nid.' '.$result;

}




 
/**************************************************************************************************************************
 * 					END SESSION
 * ***********************************************************************************************************************/ 

 /**************************************************************************************************************************
 *                  CUBRICK_USER
 * ***********************************************************************************************************************/ 

 
 function _wsmc_cubrikuser_retrieve($id_application,$id_user)
{
    
    $query = "SELECT nid as nid FROM {node} AS n 
                    LEFT JOIN 
                    {field_data_field_app_id} f1 ON
                    n.nid = f1.entity_id 
                     LEFT JOIN {field_data_field_app_user_id} f2 ON n.nid = f2.entity_id 
               WHERE type = 'user' and f1.field_app_id_value = '".$id_application."'
                     and f2.field_app_user_id_value = '".$id_user."' LIMIT 0,1"; 
    $result = db_query($query);
                    
       
   if($result->rowCount()==0) return t(print_error_string('not_valid','application id or user id'));
            
        // if there are records just grab the content_id value
        foreach ($result as $record) {
            $cubrikuser_node = node_load($record->nid);
        }
        
    $output['cubrik_userid'] = $record->nid;
    $output['app_id'] = $cubrikuser_node->field_app_id['und'][0]['value'];
    $output['app_user_id'] = $cubrikuser_node->field_app_user_id['und'][0]['value'];    

    return print_user_data($record->nid);
}


/*
 *  verifica se un utente appartiene ad una applicazione 
 * 
 */

function _wsmc_cubrikuser_index($id_application)
{
    $query = "SELECT nid as nid FROM node AS n 
                    LEFT JOIN 
                    field_data_field_app_id f1 ON
                    n.nid = f1.entity_id 
              WHERE type = 'user' 
                    and f1.field_app_id_value = '".$id_application."'
                    ";
     $result = db_query($query);
     
      //if($result->rowCount()==0) $output = t(print_error_string('not_valid','application id',$id_application));
     
     foreach ($result as $record) {
        $cubrikuser_node = node_load($record->nid);
		$user = array();
		// load user node
		$user_node = node_load($record->nid);
		// load user data
		$user['cubrik_userid'] = $user_node->nid;
		$user['app_id'] = $user_node->field_app_id['und'][0]['value'];		
		$user['app_user_id'] = $user_node->field_app_user_id['und'][0]['value'];
		
		$plays = db_query('SELECT COUNT(S.entity_id) AS NumberOfPlays
					  FROM {field_data_field_session_user} AS S
					  WHERE S.field_session_user_nid = :id_user', array(':id_user' => $user_node->nid));
		
		foreach ($plays as $numPlays) {
			$user['number_of_plays'] = $numPlays->NumberOfPlays;
		}
		
		$annotations = db_query('SELECT COUNT(A.entity_id) AS NumberOfAnn
					  FROM {field_data_field_action_user_id} AS A
					  WHERE A.field_action_user_id_nid = :id_user', array(':id_user' => $user_node->nid));
		
		foreach ($annotations as $numAnn) {
			$user['number_of_annotations'] = $numAnn->NumberOfAnn;
		}
 
        $output[] = $user;
     }
     
     return $output;
}


/*********************************************************************************************************
 *                  END CUBRICK_USER
 * ******************************************************************************************************/ 
  
 

 /*********************************************************************************************************
 *                  COLLECTION
 * ******************************************************************************************************/ 
  
 
     


/**
 * Callback for creating collection resources.
 *
 * @param object $node
 * @return object
 */
function _wsmc_collection_create( $data) {
  
  //Verifico che i campi obbligatori vengano inviati
  if(!empty($data['name']) AND !empty($data['images'])){
    $id_images = explode(',', $data['images']);
    //Verifico se gli id sono effettivamente di immagini
    foreach($id_images AS $id){
      if(is_numeric($id)){
        $immagine = image_exist($id);
        if( ! $immagine){
          $output['error'] = 'image not found';
          return $output;
        }
        //Nel frattempo preparo i dati da salvare
        $other_fields['field_collection_image']['und'][]['nid'] = $id;
      }else{
        $output['error'] = 'image not found';
        return $output;
      }
    }

    if(isset($data['tasks'])){
		$id_tasks = explode(',', $data['tasks']);
    	//Verifico se gli id sono effettivamente di immagini
    	foreach($id_tasks AS $idt){
      		if(is_numeric($idt)){
        		$taskex = task_exist($idt);
        		if(! $taskex){
          			$output['error'][] = t(print_error_string('not found','task'));
          			return $output;
        		}
        		//Nel frattempo preparo i dati da salvare
        		$other_fields['field_collection_task']['und'][]['nid'] = $idt;
      		}else{
        		$output['error'][] = t(print_error_string('not found','task'));
        		return $output;
      		}
    	}
	}
    //Se non ci sono errori creo il nodo
    $node_type = 'collection';
    $title = $data['name'];
    $new_node = create_new_node($node_type, $title, $other_fields);
    node_save($new_node);
    $output = $new_node;
    
  }else{
    if(empty($data['name'])) $output['error'][] = t(print_error_string('required','name'));
    if(empty($data['images'])) $output['error'][] = t(print_error_string('required','images'));
  }
  
  return $output;
}


function _wsmc_collection_retrieve_images($id){
	$output=array();
	$images= array();

	$result = db_query('SELECT field_collection_image_nid as nid
				FROM {node} AS N JOIN {field_data_field_collection_image} AS I 
				ON N.nid=I.entity_id
				WHERE N.nid=:nid',array(':nid' => $id) );

	foreach ($result as $record) {
	
		$images[$record->nid]['id']=$record->nid;
		
  	}
  	return $images;

}



function _wsmc_collection_retrieve_tasks($id){
	$output=array();
	$result = db_query('SELECT field_collection_task_nid as nid
				FROM {node} AS N JOIN {field_data_field_collection_task} AS I 
				ON N.nid=I.entity_id
				WHERE N.nid=:nid',array(':nid' => $id) );

	foreach ($result as $record) {
		$output[]=$record->nid;
  	}
     return $output;
}

function _wsmc_collection_retrieve($id){
  //Verifico l'id del coll
  if(is_numeric($id)){
  	$collections= db_query('SELECT nid, title FROM {node} WHERE nid= :uid', array(':uid' => $id)); 
  	foreach($collections AS $collection){
		$output = array(
          'id' => $collection->nid,
          'name' => $collection->title,
      	);
      	$output['images'] = _wsmc_collection_retrieve_images($id);
      	$output['tasks'] = _wsmc_collection_retrieve_tasks($id);
      	return $output;
  	}
    
    $output['error'] = t(print_error_string('not_valid','id collection'));

  }else{
    $output['error'] = t(print_error_string('not_valid','id collection'));
  }
  return $output;
}


function _wsmc_collection_index(){

  //aggiungi collezione alla query
  $query="SELECT n.nid FROM {node} n WHERE n.type = 'collection'";
  $result = db_query($query);

  // Result is returned as a iterable object that returns a stdClass object on each iteration
  foreach ($result as $record) {
	$output['collections'][] = _wsmc_collection_retrieve($record->nid);
  }
  return $output;
}

function _wsmc_collection_delete($nid) {
	node_delete($nid);
  	return 'delete';
}



 
 /********************************************************************************
 *                  END COLLECTION
 * *****************************************************************************/ 
  
 
 /*********************************************************************************************************
 *                  MASK
 * ******************************************************************************************************/  

 function _wsmc_mask_retrieve($id_image, $id_textAnnotation) {
 
	global $base_root;
	global $base_path;
	
	// array per inserire i dati finali da stampare
	$final_result = array();	
	$times = db_query('SELECT MAX(UP.field_mask_updated_value) AS timestamp
								FROM {field_data_field_mask_updated} AS UP
								JOIN {field_data_field_mask_tag} AS TAG
								JOIN {field_data_field_mask_image_reference} AS IMAGE
								ON UP.entity_id = TAG.entity_id AND UP.entity_id = IMAGE.entity_id
								WHERE IMAGE.field_mask_image_reference_nid = :id_image AND TAG.field_mask_tag_nid = :id_textAnnotation ', array(':id_image' => $id_image, ':id_textAnnotation' => $id_textAnnotation));	
			
			foreach ($times as $time) {
				$timestamp = $time->timestamp;
			}

			$img = array();
			$records = db_query('SELECT T.nid AS id, Q.field_mask_quality_value AS quality, U.field_mask_updated_value AS updated, TA.field_mask_tag_nid AS tag, F.uri AS uri, S.field_mask_segmentations_value AS seg
			FROM {node} AS T
			JOIN {field_data_field_mask_quality} AS Q
			JOIN {field_data_field_mask_updated} AS U
			JOIN {field_data_field_mask_tag} AS TA
			JOIN {field_data_field_mask_image_reference} AS I
			JOIN {field_data_field_mask_image} AS MI
			JOIN {file_managed} AS F
			JOIN {field_data_field_mask_segmentations} AS S
			ON  T.nid=Q.entity_id AND T.nid=U.entity_id AND T.nid=TA.entity_id AND T.nid=I.entity_id AND T.nid=MI.entity_id AND F.fid=MI.field_mask_image_fid AND S.entity_id = T.nid
 			WHERE I.field_mask_image_reference_nid = :id_image AND TA.field_mask_tag_nid = :id_textAnnotation AND U.field_mask_updated_value = :timestamp', array(':id_image' => $id_image, ':id_textAnnotation' => $id_textAnnotation, ':timestamp' => $timestamp));																									
			
			foreach ($records as $record) {
  
				$img = array();
				$img['id'] = $record->id;
				$img['path'] = "sites/default/files/images/" . substr($record->uri, 16);
				$img['quality'] = $record->quality;
				$img['updated'] = $record->updated;
				$img['mask_tag'] = $record->tag;
				$img['segmentations'] = $record->seg;
				
  				return $img;
  			} 

  			$final_result = t(print_error_string('not_valid','mask id',$id));	

  return $final_result;
			
		
}
 
 
function _wsmc_mask_create($node) {
		global $user;
		
		//retrieve parameters
		$node = _services_arg_value($node, 'node');
		$base64image = isset($node['base64image']) ? $node['base64image'] : '';
		$quality = isset($node['quality']) ? $node['quality'] : '';
		$updated = isset($node['updated']) ? $node['updated'] : '';
		$mask_image = isset($node['mask_image']) ? $node['mask_image'] : '';
		$mask_tag = isset($node['mask_tag']) ? $node['mask_tag'] : '';
		$segmentations = isset($node['segmentations']) ? $node['segmentations'] : '';
		
		//decode image
		$base64image = str_replace(' ','+',$base64image);
		$image = base64_decode($base64image);
		
		//name of the mask image file as Mask_imageId_Tag_id
		$baseName = 'Mask_'.$mask_image.'_'.$mask_tag.'.png';
		
		if($image !== FALSE){
			
			$destination = 'public://images/'.$baseName; // a stream wrapper path where you want this image to reside on your file system including the desired filename
			
			$file = file_save_data($image, $destination, FILE_EXISTS_RENAME);
			if (is_object($file)) { // if you get back a Drupal $file object, everything went as expected so make the status permenant
			   $file->status = 1;
			   $file = file_save($file);
			}
			$newNode = (object) array('type' => 'mask');
			node_object_prepare($newNode);
		  	$newNode->type = 'mask';
		  	$newNode->title = isset($node['title']) ? $node['title'] : 'Mask';
		  	$newNode->uid = 0;
		  	$newNode->comment = 0;
		  	$newNode->promote = 0;
		  	$newNode->moderate = 0;
		  	$newNode->sticky = 0;
		  	$newNode->language = 'und';
			
			$newNode->field_mask_image['und'][0] = (array)$file;
			$newNode->field_mask_quality['und'][0]['value'] = $quality;
			$newNode->field_mask_updated['und'][0]['value'] = $updated;
			$newNode->field_mask_image_reference['und'][0]['nid'] = $mask_image;
			$newNode->field_mask_tag['und'][0]['nid'] = $mask_tag;
			$newNode->field_mask_segmentations['und'][0]['value'] = $segmentations;
			
			// Save the node.
	    	node_submit($newNode);  
	    	node_save($newNode); 
		
			$output = $newNode;
		}
		else{
			$output['error'] = t(print_error_string('not_valid','File'));
		}
	return (object)$output;
}

/**
 * Callback for updating mask object.
 *
 * @param int $id_mask
 * @param object $node
 */
function _wsmc_mask_update($id_mask, $node){

	global $user;
	
  	//Verifico l'id del task
  	if(is_numeric($id_mask)){

  		$result=db_query('SELECT field_mask_image_fid
				FROM {field_data_field_mask_image} 
				WHERE entity_id= :id', array(':id' => $id_mask));
		
    	foreach($result AS $key){
    		$mask=$key;
    	}
		
	
    	if(! isset($mask)){
      		$output['error'] = t(print_error_string('not_found','id mask'));
      		return $output;
    	}
		
  	}else{
    	$output['error'] = t(print_error_string('not_valid','id mask'));
    	return $output;
  	}

		//retrieve parameters
		$node = _services_arg_value($node, 'node');
		$base64image = isset($node['base64image']) ? $node['base64image'] : '';
		$quality = isset($node['quality']) ? $node['quality'] : '';
		$updated = isset($node['updated']) ? $node['updated'] : '';
		$segmentations = isset($node['segmentations']) ? $node['segmentations'] : '';

		//decode image
		$base64image = str_replace(' ','+',$base64image);
		$image = base64_decode($base64image);
		
		//name of the mask image file as Mask_imageId_Tag_id
		$result=db_query('SELECT I.field_mask_image_reference_nid AS imageId, T.field_mask_tag_nid AS tagId
				FROM {field_data_field_mask_image_reference} AS I
				JOIN {field_data_field_mask_tag} AS T
				ON I.entity_id=T.entity_id
				WHERE I.entity_id= :id', array(':id' => $id_mask));
				
    	foreach($result AS $key){
    		$mask_image = $key -> imageId;
			$mask_tag = $key -> tagId;
    	}
		
		$baseName = 'Mask_'.$mask_image.'_'.$mask_tag.'.png';
		
		if($image !== FALSE){
			
			$destination = 'public://images/'.$baseName; // a stream wrapper path where you want this image to reside on your file system including the desired filename
			
			$file = file_save_data($image, $destination, FILE_EXISTS_REPLACE);
			if (is_object($file)) { // if you get back a Drupal $file object, everything went as expected so make the status permenant
			   $file->status = 1;
			   $file = file_save($file);
			}
			
			$node = node_load($id_mask);
    
			$node->field_mask_quality['und'][0]['value'] = $quality;
			$node->field_mask_updated['und'][0]['value'] = $updated;
			$node->field_mask_image['und'][0] = (array)$file;
			$node->field_mask_segmentations['und'][0]['value'] = $segmentations;
     
			node_save($node);
		
			$output = $node;
		}
		else{
			$output['error'] = t(print_error_string('not_valid','File'));
		}
    
  	return $output;
}
 
  /********************************************************************************
 *                  END MASK
 * *****************************************************************************/ 