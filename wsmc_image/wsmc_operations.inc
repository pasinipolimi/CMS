<?php




/* STATUS

print_user_data  usata da _wsmc_action_retrieve, da eliminare in futuro
print_language_data  usata da _wsmc_textAnnotation_retrieve
verify_language  usata da save_annotation  save_segmentation
print_error_string ok
create_new_node ok
require_fields ok
verify_node_type ok
get_annotation_id ok
add_new_content c'è un NL per modificare il titolo... mah
insert_new_annotation svae_seg/ann c'è un NL per modificare il titolo... mah
create_action c'è un NL per aggiormare la sessione ma boh
insert_point  save_segmentation c'è un NL per aggiormare la sessione ma boh
retrieveUTaskByTaskId ok
retrieveUTaskDataByTaskId non usata ma 'era per un modtivo
verify_cubrik_user mah
process_cubrik_user mah
insert_cubrik_user mah
get_consumer_by_key mah

session_retrieve_users ok
session_retrieve_actions ok
session_verify ok
image_retrieve_available_tags ok
image_retrieve_tags ok
image_retrieve_annotations ok
task_retrieve_images ok
image_exist ok
task_exist ok
_wsmc_quality_retrieve ok
save_annotation_availableTags ok
image_contains_tags ok

*/



 
/********************************************************************************
 *          FUNCTIONAL
 * *****************************************************************************/ 



/**
 * FUNZIONE CHE NORMALIZZA LA STAMPA DEI DATI DEL UTENTE
 * 
 * @param $node node: nodo corrispondente alla utente
 * @param $user_field string: qualle e il campo da interogare per prendere l'utente
 * @return $user viene ristituito un array con i dati del user.
 */

function print_user_data($node_id)
	{
		$user = array();
		
		// load user node
		$user_node = node_load($node_id);

		// load user data
		$user['cubrik_userid'] = $user_node->nid;
		$user['app_id'] = $user_node->field_app_id['und'][0]['value'];		
		$user['app_user_id'] = $user_node->field_app_user_id['und'][0]['value'];

		return $user; 

	}



/**
 * FUNZIONE CHE NORMALIZZA LA STAMPA DEI DATI DELLA LINGUA
 * 
 * @param $node node: nodo corrispondente alla lingua
 * @param $language_field string: qualle e il campo da interogare per prendere la lingua
 * @return $lang viene ristituito un array con i dati della lingua.
 */

function print_language_data($lang_id)	{
		
	$language = taxonomy_term_load($lang_id);
	$lang = $language->field_iso_code['und'][0]['value'];		

	return $lang; 
}


/*
 * verify if an input string is a write iso_code for a language
 * @param $iso_code string: the iso_code language
 * @return id of the language or 0 if no found
 */
function verify_language($iso_code)
	{
		// carrichiamo tutte le lingue
		$languages = taxonomy_get_tree(6);
		foreach($languages as $language)
			{
				$lang = taxonomy_term_load($language->tid);
				if($lang->field_iso_code['und'][0]['value']==$iso_code) return $language->tid;
					
			}
		return 0;
	}	


/*
 * return a error string in function of an error id
 * 
 * @param $special_string string: can be used when we need to use only one case with different values
 * @param $special_string_id : gives the value of the attribute associated to the special-string
 */
function print_error_string($id,$special_string=NULL,$special_string_id=NULL)
	{ 
		switch ($id){
			case 'not_valid': $err_output = $special_string.' '.$special_string_id.' is not valid';
				break;
			case 'not_found': $err_output = $special_string.' '.$special_string_id.' not found';
				break;
			case 'required': $err_output = 'Required Fields: '.$special_string.' !!!';
				break;
			default: $err_output = "error";
		}
	return $err_output;
	}

	

/*
 * @param $node_type string: define the type of the node
 * @param $other_fields array(): array di tutti i campi aggiuntivi che possono essere diversi da un tipo di contenuto all'altro
 * es. $other_fields['field_session_application_id']['und'][0]['value'] = 'Prova';
 * @param $return $node with basic values of a node
 */	
function create_new_node($node_type, $title = NULL, $other_fields =  NULL)
	{
		$node = (object) NULL;
		$node->type = $node_type;
		$node->uid = 0;
		$node->created = strtotime("now");
		$node->changed = strtotime("now");
		$node->status = 1;
		$node->comment = 0;
		$node->promote = 0;
		$node->moderate = 0;
		$node->sticky = 0;
		$node->language = 'und';
        if(!empty($title)) $node->title = $title;
        if(!empty($other_fields)){
          foreach($other_fields AS $key => $value){
            $node->$key = $value;
          }
        }
		
		return $node;
	}	
	
	
	

 
/********************************************************************************
 * 					END FUNCTIONAL
 * *****************************************************************************/  
 
 
 
/*
 * function that generalize all required fields to pass to a url. If some field is missing the error array returns
 * 
 * @fields_to_ceck string: string of fields to ceck
 * @fields_values array: values of the fields to ceck
 * 
 * */ 
 
 function require_fields($fields_to_ceck,$fields_values)
 {
     $fields_array = explode(",", $fields_to_ceck);
     
     //print_r($fields_values); exit();
     $required_field=array();
     
     for($i=0;$i<count($fields_array);$i++)
     {
         $value = $fields_array[$i]; //print_r($value); exit();
         switch ($value) {
            case "image_id":
                if($fields_values['image_id'] == NULL) $required_field[] = 'image_id';
                break;
            case "user_id":
                if($fields_values['user_id'] == NULL) $required_field[] = 'user_id';
                break;
            case "language":
                if($fields_values['language'] == NULL) $required_field[] = 'language';
                break;
            case "session_id":
                if($fields_values['session_id'] == NULL) $required_field[] = 'session_id';
                break;
            case "ta_name":
                if($fields_values['ta_name'] == NULL) $required_field[] = 'ta_name';
                break;				
			case "ta":
                if($fields_values['ta_val'] == NULL && $fields_values['ta_id'] == NULL ) $required_field[] = 'ta_val/ta_id';
                break;    
			case "content_type":
                if($fields_values['content_type'] == NULL ) $required_field[] = 'content_type';
                break;
            case "oauth_consumer_key":
                if($fields_values['oauth_consumer_key'] == NULL ) $required_field[] = 'oauth_consumer_key';
                break;                				
            case "polyline":
                if($fields_values['polyline_r'] == NULL && $fields_values['polyline_h'] == NULL)	$required_field[] = 'polyline_r/polilyne_h';
                break; 
            case "action_id":
                if($fields_values['action_id'] == NULL) $required_field[] = 'action_id';
                break;
            case "action_log":
                if($fields_values['action_log'] == NULL) $required_field[] = 'action_log';
                break;
            case "action_type":
                if($fields_values['action_type'] == NULL) $required_field[] = 'action_type';
                break;
            case "state":
                if($fields_values['state'] == NULL) $required_field[] = 'state';
                break;                 
				        }
     }

    if(count($required_field)>0)    return $required_field; 
    else return 0;
 }
 
 
 /*
  * function that verify if a node is of the type that we desire
  * 
  * @node_id int: id of the node 
  * @desired_type string: the value of the type that we want to ceck
  * 
  * return 0 if everything is ok
  * return 1 if not ok
  *  
  */
function verify_node_type($node_id,$desired_type,$field_annotation_content=NULL) {


	$res=db_query('SELECT nid 
				FROM {node} AS I
				WHERE type = :t AND nid = :uid', array(':uid' => $node_id, ':t'=> $desired_type));
	foreach($res AS $key){
        return 1;
    }
    return 0;	

 }

 /*
  * function that retrieve the id of the annotatiion in function of the content type and the value
  * 
  *  @annotation_value string: 	the value of tha annotation we whant to find the id
  *  @language_type	string:		the type of the annotation
  *  @language_id	string:		the id of the language
  * 
  *  @return int:				return 0 if annotation not found, return the id of annotation if annotation fount
  */
 
 function get_annotation_id($annotation_value,$annotation_name,$language_id)
 {
     $result = db_query("SELECT N.nid AS id FROM node AS N
		JOIN {field_data_field_annotation_value} AS V
		JOIN {field_data_field_annotation_name} AS NA
		JOIN {field_data_field_annotation_language} AS L
		ON N.nid=V.entity_id AND N.nid=V.entity_id AND N.nid=NA.entity_id AND N.nid=L.entity_id 
		WHERE type = 'textannotation'AND V. field_annotation_value_value= :v AND NA. field_annotation_name_value= :n AND L. field_annotation_language_tid=:l", array(':v' => $annotation_value, ':n'=> $annotation_name, ':l'=>$language_id));
     

        foreach ($result as $record) {
			return $record->id;
		}
        
     return 0;
 }



/*
 *  function add a new content 
 * 
 *  @content_type string	type of the conten (name)
 *  @img_id node			the node of the image being saved
 *  @annotation_reff node	the foreign key for the text annotatio
 *  @point	node array 		the list of point
 * 
 *  return 					we will see
 */
function add_new_content($content_type,$img_id,$annotation_reff=NULL,$points=NULL)
{
	
	//$content_id = get_content_id_from_type($content_type);
	

    $node = create_new_node('content');
    
    $node->field_content_name['und'][0]['value'] = $content_type;
    
    $node->field_content_textannotation['und'][0]['nid'] = $annotation_reff;
   
    $node->field_content_point['und'] = $points;
      
    $node->field_content_image['und'][0]['nid'] = $img_id;

    //$node->field_content_id['und'][0]['value'] = $content_id;
	
	  $node->field_content_valid['und'][0]['value'] = 1;
        
    node_save($node);
    
    $node = node_load($node->nid);
    // stamp the title
    $node->title = 'Content '.$node->nid.' ( '.$content_type.' )';

    // if no content_id was found update it from 1 to actual nid
    if($content_id==1) $node->field_content_id['und'][0]['value'] = $node->nid;
	
    node_save($node);  
    
    return $node->nid;
}

 
 /*
  * functionthat inserts a new annotation
  * 
  * @annotation_name:   the name of the annotation (example: tag,segment,vote)
  * @language_id:       the id of the language
  * @criteria_id:       criteria id if any
  * @range_id:          range id if any
  * @content_id:        content id 
  * @value string:      the value of the annotation
  * 
  */
 function insert_new_annotation($value,$language_id,$annotation_name,$criteria_id = NULL,$range_id = NULL)
 {
    $node = create_new_node('textannotation');
        
    $node->field_annotation_value['und'][0]['value'] = $value;
    
    $node->field_annotation_name['und'][0]['value'] = $annotation_name;
	
	//imposto di default l'annotazione come valida quando viene creata
	//$node->field_annotation_valid['und'][0]['value'] = 1;
    
    // add the content id
    //$node->field_annotation_content['und'][0]['nid'] = $content_id;
    // add the language id
    $node->field_annotation_language['und'][0]['tid'] = $language_id;
     
    node_save($node);
    
    $node = node_load($node->nid);
    // stampo il title
 
    $node->title = 'Annotation '.$node->nid.' ( '.$node->field_annotation_value['und'][0]['value'].' )';
    node_save($node);  
	return $node->nid;
 }
 
 /*
  *	function that creates a new action
  * 
  * @session_id	int:		the id of the session
  * @user_id int:       	the id of the user
  * @action_type string:    the type of the action (tagging, segmentation, commenting ect)
  * @action_log string:     the log of the action... ?
  * @image_id int:        	the id of the image
  * @content_id int:        the id of the content description
  * @annotation_id int:     the id of the annotation
  * @point_id int:     		the id of the point
  * 
  * @retur int				the id of the new created action
  * 
  */
 function create_action($session_id,$user_id,$action_type,$action_log=NULL,$image_id=NULL,$content_id=NULL,$annotations=NULL,$points=NULL)
 {
 	$timestamp = time();
		
    $node = create_new_node('action');
        
    $node->field_action_type_val['und'][0]['value'] = $action_type;
    
    $node->field_action_log_val['und'][0]['value'] = $action_log;
    
    // add the session id
    $node->field_action_session_id['und'][0]['nid'] = $session_id;
    
    // add the user id
    $node->field_action_user_id['und'][0]['nid'] = $user_id;

    // add the image id
    $node->field_action_image_id['und'][0]['nid'] = $image_id;
	
    // add the content id
    $node->field_action_content_id['und'][0]['nid'] = $content_id;

    // add the content type
    //$node->field_action_content_type['und'][0]['value'] = $content_type;
	
    // add the annotation id
    $node->field_action_annotation_id['und'][]['nid'] = $annotations;
	
    // add the point id
    $node->field_action_point_id['und'] = $points;			        
      
    // add the startts value
    $node->field_action_startts_val['und'][0]['value'] = $timestamp;
	
    // add the endts value
    $node->field_action_endts_val['und'][0]['value'] = time();  
	  
    node_save($node);
    
    $node = node_load($node->nid);
    $action_id = $node->nid;
    // stampo il title
    $node->title = 'Action '.$node->nid;
    node_save($node);  
    
    // riferimento della action dentro la session
	
    $session_node = node_load($session_id);
    $session_node->field_session_actions['und'][]['nid']=$action_id;

    // update user list	
    update_session_user_list($session_id,$user_id);
	
    return $node; 	
 }

/*
 * function that update the user list of the session in the case that a new user is added
 * in the case the user already exist function does'nt add anything to the session
 *  
 */
function update_session_user_list($session_id,$user_id)
{
    $session_node = node_load($session_id);
    
        // update user list 
    $user_list = $session_node->field_session_user['und'];
    
    $unic_user = 1; 
    
    foreach ($user_list as $key => $user) {
        if($user_id==$user['nid']) $unic_user = 0;
    }
    
    if ($unic_user) $session_node->field_session_user['und'][]['nid'] = $user_id;
    
    node_save($session_node);
    
    return;
}


 /*
  *	function that creates a new point
  * 
  * @point_value array:		the value of the point
  * @point_name string:    	the name of the point
  * @content_id int:        the id of the content description
  * 
  * @retur int				the id of the new created point
  * 
  */
function insert_point($point_value,$point_name)
	{
		
	$node = create_new_node('point');
        
    $node->field_point_value['und'][0]['value'] = $point_value;
    
    $node->field_point_name['und'][0]['value'] = $point_name;

    node_save($node);
    
    $node = node_load($node->nid);
    // stampo il title
    $node->title = 'Point '.$node->nid;
    node_save($node);  
    
    return $node->nid;		
	}

function retrieveUTaskByTaskId($id_task){
  $id = array();
  $result = db_query('SELECT f.entity_id AS task_id, S.field_utask_status_value AS status, T.field_utask_type_value AS type
					FROM {field_data_field_utask_task} AS f 
					JOIN {field_data_field_utask_status} AS S
					JOIN {field_data_field_utask_type} AS T
					ON f.entity_id = S.entity_id AND f.entity_id = T.entity_id
					WHERE f.field_utask_task_nid = :taskid', array(':taskid' => $id_task));
  // Result is returned as a iterable object that returns a stdClass object on each iteration
  foreach ($result AS $record) {
    $id[$record->task_id]['id'] = $record->task_id;
    $id[$record->task_id]['status'] = $record->status;
    $id[$record->task_id]['utaskType'] = $record->type;
	}
  return $id;
  
}



function retrieveUTaskDataByTaskId($id_task){
	$utasks = array();
  	$result = db_query('SELECT N.title AS title, S. field_utask_status_value AS status, N.nid AS id 
  		FROM {field_data_field_utask_task} AS F 
  		JOIN {field_data_field_utask_status} AS S 
  		JOIN {node} AS N
		ON F.entity_id=S.entity_id AND N.nid=S.entity_id 
		WHERE f.field_utask_task_nid = :taskid', array(':taskid' => $id_task));
  	// Result is returned as a iterable object that returns a stdClass object on each iteration
  	foreach ($result as $record) {
    	$id[] = $record->task_id;
    	$utasks = array(
        	'id' => $record->id,
        	'taskType' => $record->title,
        	'status' => $record->status,
      	);
	  
		$utasks[] = $utasks; 
		unset($utasks);
  	}

  return $utasks;
}


/*
 *  function that verify if a cubrik_user with specifiec id_application and id_user already exists. 
 * if so cubrik_user_id is returned. if not 0 is returned  
 * 
 */

function verify_cubrik_user($id_application,$id_user)
{
    
        $result = db_query("SELECT nid as nid FROM node AS n 
                    LEFT JOIN 
                    field_data_field_app_id f1 ON
                    n.nid = f1.entity_id
                    LEFT JOIN
                    field_data_field_app_user_id f2 ON
                    n.nid = f2.entity_id
                     
                    WHERE type = 'user' 
                    and f1.field_app_id_value = '".$id_application."'
                    and f2.field_app_user_id_value = '".$id_user."'
                    LIMIT 0,1");

        
       if($result->rowCount()==0) return 0;
            
        // if there are records just grab the cubrik_user_id
        foreach ($result as $record) {
            $cubrick_user_id = $record->nid;
        }
        
    return $cubrick_user_id;
}

/*
 *  function that control the cubrik user.
 * first we control if a user exists. if so return its id
 * if not, insert new user  
 * 
 */

function process_cubrik_user($id_application,$id_user)
{
    $controled = verify_cubrik_user($id_application,$id_user);
    
    if(!$controled) $cubrick_user_id = insert_cubrik_user($id_application, $id_user);
    else $cubrick_user_id = $controled;
    
    return $cubrick_user_id;
}


/*
 * function that inserts a new cubrik user
 * the cubrik user id is returned
 * 
 */
function insert_cubrik_user($id_application,$id_user)
{
        // creo il nuovo nodo
        $node = create_new_node('user');

        // valorifico lo stato
        $node->field_app_id['und'][0]['value'] = $id_application;
        
        $node->field_app_user_id['und'][0]['value'] = $id_user;
    
        node_save($node);
        
        $node = node_load($node->nid);
        // stampo il title
        $node->title = 'User '.$node->nid;
        node_save($node);
                
        // stampo il valore della nuova sessione
        return $node->nid;
}

function get_consumer_by_key($consumer_key){
  $consumer_result = db_query('SELECT o.csid, o.uid, o.name FROM {oauth_common_provider_consumer} o WHERE o.consumer_key = :ckey', array(':ckey' => $consumer_key));
  foreach ($consumer_result as $consumer_record) {
    $consumer_details['id'] = $consumer_record->csid;
    $consumer_details['key'] = $consumer_key;
    $consumer_details['name'] = $consumer_record->name;
    $consumer_details['user'] = $consumer_record->uid;
  }
  return $consumer_details;
}



function session_retrieve_users($id){
	$output=array();

	$res_users=db_query('SELECT SU.field_session_user_nid AS id, AU.field_app_user_id_value AS app_usr_id, A.field_app_id_value AS app_id
				FROM {field_data_field_session_user} AS SU
				JOIN {field_data_field_app_id} AS A
				JOIN {field_data_field_app_user_id} AS AU ON SU.field_session_user_nid = A.entity_id
				AND SU.field_session_user_nid = AU.entity_id
				WHERE SU.entity_id = :i', array(':i' => $id));



	foreach ($res_users as $res_user) {
    	if(isset($res_user->id)){
        	$user = array();
			$user['cubrik_userid'] = $res_user->id;
			$user['app_id'] = $res_user->app_id;		
			$user['app_user_id'] = $res_user->app_usr_id;
			$output[] = $user;
			unset($user);
		}
    }

	return $output;
}


function session_retrieve_actions($id){
	$output=array();

	$res_acts=db_query('SELECT *
						FROM  {field_data_field_session_actions} AS S
						JOIN {actionsV} AS A
						ON A.action_id=S.field_session_actions_nid
						WHERE S.entity_id= :nid', array(':nid' => $id)); 
			
	foreach ($res_acts  as $action)
	{		
    	$outputa['id'] = $action->action_id;
    	$outputa['startTs'] = $action->start;
		  $outputa['endTs'] = $action->end;    
		  $outputa['type'] = $action->type;    
		  $outputa['user'] = $action->user;
		  $outputa['contentid'] = $action->content;
		  $outputa['imageid'] = $action->image;
		  $outputa['quality'] = _wsmc_quality_retrieve(1);
		  $output['actions'][] = $outputa;	
	}	
	return $output;
}


/*
 * function that verify if a session exists and if it is opened
 * 
*/

function session_verify($session_id)
{
	$res=db_query('SELECT N.nid
                  FROM {node} AS N
                  WHERE N.nid = :id', array(':id' => $session_id));

  foreach($res AS $key){
      return 1;
  }
  $output = t('Session id '.$session_id.' does not exist');
  return $output;
}


function image_retrieve_available_tags($id){
	
	$content_query = db_query("SELECT A.entity_id AS id, field_annotation_value_value AS value, ISO.field_iso_code_value as lang
    FROM {field_data_field_content_name} as C 
    JOIN {field_data_field_content_textannotation} as A 
    JOIN {field_data_field_annotation_value} AS V
    JOIN {field_data_field_content_valid} AS VA
    JOIN {annotations} AS ASS
    JOIN {field_data_field_annotation_language} AS L
    JOIN {field_data_field_iso_code} AS ISO
    ON A.entity_id=C.entity_id AND A.field_content_textannotation_nid=V.entity_id AND C.entity_id = ASS.contentid AND V.entity_id=L.entity_id AND C.entity_id = VA.entity_id AND ISO.entity_id=L.field_annotation_language_tid
    WHERE C.field_content_name_value = 'availableTags' AND VA.field_content_valid_value = 1 AND imageid = :uid", array(':uid' => $id));

	$tags = array();
  foreach ($content_query as $content_record) {
		$tags[]=$content_record;  	
  }
  	
  return $tags;

}

function image_contains_tags($id, $tag, $language){
  
 
 

  $content_query = db_query("SELECT A.entity_id AS id, field_annotation_value_value AS value, ISO.field_iso_code_value as lang
    FROM {field_data_field_content_name} as C 
    JOIN {field_data_field_content_textannotation} as A 
    JOIN {field_data_field_annotation_value} AS V
    JOIN {field_data_field_content_valid} AS VA
    JOIN {annotations} AS ASS
    JOIN {field_data_field_annotation_language} AS L
    JOIN {field_data_field_iso_code} AS ISO
    ON A.entity_id=C.entity_id AND A.field_content_textannotation_nid=V.entity_id AND C.entity_id = ASS.contentid AND V.entity_id=L.entity_id AND C.entity_id = VA.entity_id AND ISO.entity_id=L.field_annotation_language_tid
    WHERE C.field_content_name_value = 'availableTags' AND VA.field_content_valid_value = 1 AND imageid = :uid AND  
  field_annotation_value_value= :t AND ISO.field_iso_code_value=:l", array(':uid' => $id, ':t'=> $tag, ':l'=>$language));

  $tags = array();
  foreach ($content_query as $content_record) {
    return 1; 
  }
    
  return 0;

}



function image_retrieve_tags($id){

  	$content_query = db_query("SELECT T.entity_id AS id, A.field_annotation_value_value as annotation_value
								FROM textannotation AS T
								JOIN {field_data_field_content_textannotation} AS C
								JOIN {field_data_field_annotation_value} AS A
								ON T.entity_id = C.entity_id AND C.field_content_textannotation_nid = A.entity_id
								WHERE imageid = :uid", array(':uid' => $id));

	$tags = array();
    foreach ($content_query as $content_record) {
		$tags[]=$content_record;
    	
  	}
  	return $tags;
}


function image_retrieve_annotations($id){
	$segmentations = db_query("SELECT entity_id AS id FROM {segmentation} WHERE imageid = :uid", array(':uid' => $id));
	$annot = array();
	foreach ($segmentations as $segm) {
			$annot[] = $segm;
	}
	return $annot;
}

function task_retrieve_images($id){
	$images=array();
	$res=db_query('SELECT I. field_task_image_nid AS img 
				FROM {field_data_field_task_image} AS I
				WHERE entity_id = :uid', array(':uid' => $id));
	foreach($res AS $key){
        $images[$key->img]['id'] = $key->img;
    }
    return $images;
}

function image_exist($id){
	
	$res=db_query('SELECT nid 
				FROM {node} AS I
				WHERE type = :t AND nid = :uid', array(':uid' => $id, ':t'=> 'image'));
	
	
	foreach($res AS $key){
        return 1;
    }
    
    return 0;	
}

function task_exist($id){
	$res=db_query('SELECT nid 
				FROM {node} AS I
				WHERE type = :t AND nid = :uid', array(':uid' => $id, ':t'=> 'task'));
	foreach($res AS $key){
        return 1;
    }
    return 0;	
}

function _wsmc_quality_retrieve($id) {
	return $id;
}


function save_annotation_availableTags($image_id, $session_id, $language_id, $cubrik_user_id, $ta_val, $ta_name )
{
  
  $content_type = 'availableTags';

  
  // find if this kind of the ta value is presented already
  $ta_id = get_annotation_id($ta_val,$ta_name,$language_id);
  // if the value was not found we insert a new annotation
  if($ta_id==0) $ta_id = insert_new_annotation($ta_val,$language_id,$ta_name);
    

  //add new tag content to contents
  $content_id = add_new_content($content_type,$image_id,$ta_id);

  $action = create_action($session_id,$cubrik_user_id,'annotation',NULL,$image_id,$content_id,$ta_id,NULL);
  
  return $action;

}



?>