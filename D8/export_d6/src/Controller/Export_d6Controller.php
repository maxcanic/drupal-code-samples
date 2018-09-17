<?php
/**
 * Created by PhpStorm.
 * User: nemanja
 * Date: 3.1.17.
 * Time: 11.28
 */
namespace Drupal\export_d6\Controller;

use Drupal\Core\Controller\ControllerBase;
use Symfony\Component\HttpFoundation\Request;
use \Drupal\node\Entity\Node;
use Drupal\Core\Field\FieldItemList;
use Drupal\file\Entity\File;
use Drupal\taxonomy\Entity\Term;
use \Drupal\Component\Utility\NestedArray;

class Export_d6Controller extends ControllerBase {

  public function content() {
    return array(
      '#type' => 'markup',
    );
  }

  public function export_d6_import() {
    $source_folder = 'blog_image_source/';

    $authors = Export_d6Controller::export_d6_get_author_terms();


    // Switch to external database
    \Drupal\Core\Database\Database::setActiveConnection('ddd6');

    // Get a connection going
    $db = \Drupal\Core\Database\Database::getConnection();

    $query = $db->query("SELECT
                         n.nid,
                         n.title,
                         n.created,
                         nr.body,
                         tn.tid,
                         td.vid,
                         u.dst,
                         ctb.field_blog_callout_value,
                         ctb.field_blog_guest_author_value AS author_1,
                         ctb.field_blog_guest_author_2_value AS author_2,
                         ctb.field_blog_guest_author_3_value AS author_3,
                         ctb.field_blog_image_fid,
                         f.filename,
                         f.filepath,
                         f.filemime

                         FROM node n
                         INNER JOIN node_revisions nr ON n.vid = nr.vid
                         JOIN content_type_blog ctb ON n.nid = ctb.nid
                         JOIN term_node tn ON n.nid = tn.nid
                         JOIN term_data td ON tn.tid = td.tid
                         JOIN files f ON ctb.field_blog_image_fid = f.fid
                         INNER JOIN url_alias u ON u.src LIKE CONCAT('node/', n.nid)
                         WHERE n.status = 1
                         ")->fetchAll();



    /*
     * Content type Blog export.
     */
    \Drupal\Core\Database\Database::setActiveConnection('default');
    $db8 = \Drupal\Core\Database\Database::getConnection();

    foreach($query as $key => $val) {
      $nodes[$val->nid]['terms'][$val->vid][] = $val->tid;
      $nodes[$val->nid]['data'] = $val;
    }

//    kint($nodes);
    $field_authors = array();
    // Node Blog save.
    foreach($nodes as $nid => $values) {

      $field_solutions = array();
      $field_country_region = array();

      if(!empty($values['terms'][1])) {
        foreach ($values['terms'][1] as $term_solution) {
          $field_solutions[]['target_id'] = $term_solution;
        }
      }

      if(!empty($values['terms'][3])) {
        foreach ($values['terms'][3] as $term_region) {
          $field_country_region[]['target_id'] = $term_region;
        }
      }

//      $i++;
//      //drupal_set_message($i);
//      if($i > 2) {
//        drupal_set_message(222);
//        return;
//      }
//      drupal_set_message(print_r( $values['data']->author_1, 1));
//      drupal_set_message(print_r( $values['data']->author_2, 1));
//      drupal_set_message(print_r( $values['data']->author_3, 1));
//      drupal_set_message('-------------------------');

      if(!empty($values['data']->author_1) || !empty($values['data']->author_2) || !empty($values['data']->author_3)) {
        if(!empty($values['data']->author_1) && !empty($authors[$values['data']->author_1])) {
          $field_authors[]['target_id'] = $authors[$values['data']->author_1];
        }
        if(!empty($values['data']->author_2) && !empty($authors[$values['data']->author_2])) {
          $field_authors[]['target_id'] = $authors[$values['data']->author_2];
        }
        if(!empty($values['data']->author_3) && !empty($authors[$values['data']->author_3])) {
          $field_authors[]['target_id'] = $authors[$values['data']->author_3];
        }
      } else {
        drupal_set_message(print_r( $values['data']->author_1, 1));
        drupal_set_message(print_r( $values['data']->author_2, 1));
        drupal_set_message(print_r( $values['data']->author_3, 1));
      }

      $date = $values['data']->created;
      $fid = $values['data']->field_blog_image_fid;
      $file_path = $values['data']->filepath;
      $file_name = $values['data']->filename;
      $file_mime = $values['data']->filemime;
      $alias = $values['data']->dst;
//      $field_solutions = (!empty($values['terms'][1])) ? array('target_id' => $values['terms'][1]) : NULL;
//      $field_country_region = (!empty($values['terms'][3])) ?  array('target_id' => $values['terms'][3]) : NULL;
      $body = str_replace(array(
        '&rsquo;',
        '&#39;',
        '&ndash;',
        '&amp;',
        '&quot;',
        '&ldquo;',
        '<p><strong><em>',
        '</p></strong></em>'
      ),
        array(
          "'",
          "'",
          "-",
          "&",
          '"',
          "“",
          "<h2>",
          "</h2>",
        ),
        $values['data']->field_blog_callout_value . $values['data']->body
      );




      if(file_exists($source_folder . $file_name)) {
        $uri  = file_unmanaged_copy($source_folder . $file_name, 'public://' . $file_name, FILE_EXISTS_REPLACE);
        $file = File::Create([
          'uri' => $uri,
          'status' => 1,
          'filename' => $file_name,
          'uid' => 1,
        ]);
        $file->save();

        if ($file->id()) {
          $file_data = [
            'target_id' => $file->id(),
            'alt' => "",
            'title' => "",
          ];
        } else {
          $file_data = [];
        }
      } else {
        $file_data = [];
      }



      $node_export = Node::create([
        // The node entity bundle.
        'type' => 'blog',
        'langcode' => 'en',
        'created' => REQUEST_TIME,
        'changed' => REQUEST_TIME,
        // The user ID.3

        'field_solutions' => $field_solutions,

        'field_country_region' => $field_country_region,

        'field_category_reference' => [],

        'field_authors' => $field_authors,

        'field_date' => date('Y-m-d',$date),

        'field_blog_image' => [
          $file_data,
        ],

        'uid' => 1,
        'title' => $values['data']->title,

        'field_blog_body' => [
          'summary' => '',
          'value' => $body,
          'format' => 'full_html',
        ],

        'old_nid' => $values['data']->nid,

//        'field_image_mask' => 1,

      ]);
      //drupal_set_message(print_r($node_export, 1));
      $node_export->save();
      \Drupal::service('path.alias_storage')->save("/node/" . $node_export->id(), "/" . $alias, "en");
//      $i ++;
//      if($i > 10) {
//        return;
//      }
//      return;

//      kint($node_export->fields['nid']->getValue());

    }
  }

  function export_d6_term_import() {

    // Switch to external database
    \Drupal\Core\Database\Database::setActiveConnection('ddd6');

    // Get a connection going
    $db = \Drupal\Core\Database\Database::getConnection();
    $sql = '
    (SELECT DISTINCT(ctb.field_blog_guest_author_value) AS author
    FROM content_type_blog ctb
    WHERE ctb.field_blog_guest_author_value is not NULL)
    UNION
    (SELECT DISTINCT(ctb.field_blog_guest_author_2_value) AS author
    FROM content_type_blog ctb
    WHERE ctb.field_blog_guest_author_2_value is not NULL)
    UNION
    (SELECT DISTINCT(ctb.field_blog_guest_author_3_value) AS author
    FROM content_type_blog ctb
    WHERE ctb.field_blog_guest_author_3_value is not NULL)
    ';
    $query = $db->query($sql)->fetchAll();


    \Drupal\Core\Database\Database::setActiveConnection('default');
    $db8 = \Drupal\Core\Database\Database::getConnection();

    foreach($query as $result) {
     // drupal_set_message($result->author);
      $term = Term::create([
      'vid' => 'authors',
      'langcode' => 'en',
      'name' => $result->author,
      'weight' => 0,
      'parent' => array (0),
      ]);
      $term->save();
//      $i++;
//      if($i > 10) {
//        return;
//      }
    }


  }

  public function export_d6_get_author_terms() {
    \Drupal\Core\Database\Database::setActiveConnection('default');
    $db8 = \Drupal\Core\Database\Database::getConnection();

    $sql = "
    SELECT DISTINCT(name), tid FROM taxonomy_term_field_data WHERE vid = 'authors'
    ";

    $query = $db8->query($sql)->fetchAll();

    $authors = array();

    foreach ($query as $result) {
      $authors[$result->name] = $result->tid;
//      drupal_set_message(print_r($result, 1));
    }
    return $authors;
  }


  /*
   * News and Events content type import.
   */
  function export_d6_news_import() {
    // Switch to external database
    \Drupal\Core\Database\Database::setActiveConnection('ddd6');

    // Get a connection going
    $db = \Drupal\Core\Database\Database::getConnection();

    $sql = "
    SELECT n.nid,
           n.created,
	         n.title,
           nr.body,
           tn.tid,
           u.dst,
           td.vid
    FROM node n
    INNER JOIN node_revisions nr ON n.vid = nr.vid
    JOIN term_node tn ON n.nid = tn.nid
    JOIN term_data td ON tn.tid = td.tid
    INNER JOIN url_alias u ON u.src LIKE CONCAT('node/', n.nid)
    WHERE n.status = 1 AND type = 'news_event'
    ";

    $query = $db->query($sql)->fetchAll();

    \Drupal\Core\Database\Database::setActiveConnection('default');
    $db8 = \Drupal\Core\Database\Database::getConnection();

    foreach ($query as $key => $val) {
//      kint($query);
      $news[$val->nid]['terms'][$val->vid][] = $val->tid;
      $news[$val->nid]['data'] = $val;
    }
//    kint($news);



    foreach ($news as $nid => $values) {

      $field_solutions = array();
      $field_country_region = array();

      if(!empty($values['terms'][1])) {
        foreach ($values['terms'][1] as $term_solution) {
          $field_solutions[]['target_id'] = $term_solution;
        }
      }

      if(!empty($values['terms'][3])) {
        foreach ($values['terms'][3] as $term_region) {
          $field_country_region[]['target_id'] = $term_region;
        }
      }


      $date = $values['data']->created;
      $title = $values['data']->title;
      $body = $values['data']->body;
      $alias = $values['data']->dst;

//      kint($field_solutions);

      $node_export = Node::create([
        // The node entity bundle.
        'type' => 'latest_news',
        'langcode' => 'en',
        'created' => REQUEST_TIME,
        'changed' => REQUEST_TIME,
        // The user ID.3

        'uid' => 1,
        'title' => $title,

        'field_date' => date('Y-m-d',$date),

        'field_solutions' => $field_solutions,

        'field_country_region' =>  $field_country_region,

        'body' => [
          'summary' => '',
          'value' => $body,
          'format' => 'full_html',
        ],
      ]);
//      kint($news);
      $node_export->save();
      \Drupal::service('path.alias_storage')->save("/node/" . $node_export->id(), "/" . $alias, "en");
//      $i++;
//      if($i > 5) {
//        return;
//      }
    }
  }

  function export_d6_import_page() {
    // Switch to external database
    \Drupal\Core\Database\Database::setActiveConnection('ddd6');

    // Get a connection going
    $db = \Drupal\Core\Database\Database::getConnection();

    $sql = "
    SELECT n.nid,
           n.created,
	         n.title,
           nr.body,
           nr.teaser,
           re.field_tp_references_value,
           tn.tid,
           u.dst,
           td.vid
    FROM node n
    INNER JOIN node_revisions nr ON n.vid = nr.vid
    JOIN term_node tn ON n.nid = tn.nid
    JOIN term_data td ON tn.tid = td.tid
    JOIN content_field_tp_references re ON n.nid = re.nid
    INNER JOIN url_alias u ON u.src LIKE CONCAT('node/', n.nid)
    WHERE n.status = 1 AND type = 'page'
    ";

    $query = $db->query($sql)->fetchAll();

    \Drupal\Core\Database\Database::setActiveConnection('default');
    $db8 = \Drupal\Core\Database\Database::getConnection();

    foreach ($query as $key => $val) {
      $page[$val->nid]['terms'][$val->vid][] = $val->tid;
      $page[$val->nid]['data'] = $val;
    }

    foreach ($page as $nid => $values) {

      $field_solutions = array();
      $field_country_region = array();

      if(!empty($values['terms'][1])) {
        foreach ($values['terms'][1] as $term_solution) {
          $field_solutions[]['target_id'] = $term_solution;
        }
      }

      if(!empty($values['terms'][3])) {
        foreach ($values['terms'][3] as $term_region) {
          $field_country_region[]['target_id'] = $term_region;
        }
      }

      $date = $values['data']->created;
      $title = $values['data']->title;
      $body = $values['data']->body;
      $teaser = $values['data']->teaser;
      $references = $values['data']->field_tp_references_value;
      $alias = $values['data']->dst;

      $node_export = Node::create([
        'type' => 'other',
        'langcode' => 'en',
        'created' => REQUEST_TIME,
        'changed' => REQUEST_TIME,

        'uid' => 1,
        'title' => $title,

        'field_date' => date('Y-m-d',$date),

        'field_solutions' => $field_solutions,

        'field_country_region' =>  $field_country_region,

        'body' => [
          'summary' => $teaser,
          'value' => $body,
          'format' => 'full_html',
        ],
        'field_references' => $references,
      ]);
      $node_export->save();
      \Drupal::service('path.alias_storage')->save("/node/" . $node_export->id(), "/" . $alias, "en");
    }

  }

  function export_d6_import_data() {
    // Switch to external database
    \Drupal\Core\Database\Database::setActiveConnection('ddd6');

    // Get a connection going
    $db = \Drupal\Core\Database\Database::getConnection();

    $sql = "SELECT
            n.nid,
            n.title,
            n.created,
            nr.body,
            tn.tid,
            td.vid,
            u.dst,
            dl.field_document_links_url,
            dl.field_document_links_title

            FROM node n
            INNER JOIN node_revisions nr ON n.vid = nr.vid
            JOIN content_field_document_links dl ON n.nid = dl.nid
            JOIN term_node tn ON n.nid = tn.nid
            JOIN term_data td ON tn.tid = td.tid
            INNER JOIN url_alias u ON u.src LIKE CONCAT('node/', n.nid)
            WHERE n.status = 1";

    $query = $db->query($sql)->fetchAll();

    \Drupal\Core\Database\Database::setActiveConnection('default');
    $db8 = \Drupal\Core\Database\Database::getConnection();

    foreach ($query as $key => $val) {
      $data[$val->nid]['terms'][$val->vid][] = $val->tid;
      $data[$val->nid]['data'] = $val;
    }

    kint($data);

    foreach ($data as $nid => $values) {

      $field_solutions = array();
      $field_country_region = array();

      if(!empty($values['terms'][1])) {
        foreach ($values['terms'][1] as $term_solution) {
          $field_solutions[]['target_id'] = $term_solution;
        }
      }

      if(!empty($values['terms'][3])) {
        foreach ($values['terms'][3] as $term_region) {
          $field_country_region[]['target_id'] = $term_region;
        }
      }

      $date = $values['data']->created;
      $title = $values['data']->title;
      $body = $values['data']->body;
      $alias = $values['data']->dst;

      $node_export = Node::create([
        'type' => 'data',
        'langcode' => 'en',
        'created' => REQUEST_TIME,
        'changed' => REQUEST_TIME,

        'uid' => 1,
        'title' => $title,

        'field_date' => date('Y-m-d',$date),

        'field_solutions' => $field_solutions,

        'field_country_region' =>  $field_country_region,

        'body' => [
          'summary' => '',
          'value' => $body,
          'format' => 'full_html',
        ],
      ]);
      $node_export->save();
      \Drupal::service('path.alias_storage')->save("/node/" . $node_export->id(), "/" . $alias, "en");
    }
  }

  function export_d6_import_story() {
    $source_folder = 'site_images_source/';

    // Switch to external database
    \Drupal\Core\Database\Database::setActiveConnection('ddd6');

    // Get a connection going
    $db = \Drupal\Core\Database\Database::getConnection();

    $sql = "SELECT
             n.nid,
             n.title,
             n.created,
             nr.body,
             tn.tid,
             td.vid,
             u.dst,
             st.field_story_image_fid,
             f.filename,
             f.filepath,
             f.filemime

             FROM node n
             INNER JOIN node_revisions nr ON n.vid = nr.vid
             JOIN content_type_story st ON n.nid = st.nid
             JOIN term_node tn ON n.nid = tn.nid
             JOIN term_data td ON tn.tid = td.tid
             JOIN files f ON st.field_story_image_fid = f.fid
             INNER JOIN url_alias u ON u.src LIKE CONCAT('node/', n.nid)
             WHERE n.status = 1 AND n.type = 'story'
             ";

    $query = $db->query($sql)->fetchAll();

    \Drupal\Core\Database\Database::setActiveConnection('default');
    $db8 = \Drupal\Core\Database\Database::getConnection();

    foreach($query as $key => $val) {
      $story[$val->nid]['terms'][$val->vid][] = $val->tid;
      $story[$val->nid]['data'] = $val;
    }

//    kint($story);

    foreach($story as $nid => $values) {

      $field_solutions = array();
      $field_country_region = array();

      if (!empty($values['terms'][1])) {
        foreach ($values['terms'][1] as $term_solution) {
          $field_solutions[]['target_id'] = $term_solution;
        }
      }

      if (!empty($values['terms'][3])) {
        foreach ($values['terms'][3] as $term_region) {
          $field_country_region[]['target_id'] = $term_region;
        }
      }

      $date = $values['data']->created;
      $fid = $values['data']->field_blog_image_fid;
      $file_path = $values['data']->filepath;
      $file_name = $values['data']->filename;
      $file_mime = $values['data']->filemime;
      $alias = $values['data']->dst;
      $body = $values['data']->body;

      if(file_exists($source_folder . $file_name)) {
        $uri  = file_unmanaged_copy($source_folder . $file_name, 'public://' . $file_name, FILE_EXISTS_REPLACE);
        $file = File::Create([
          'uri' => $uri,
          'status' => 1,
          'filename' => $file_name,
          'uid' => 1,
        ]);
        $file->save();

        if ($file->id()) {
          $file_data = [
            'target_id' => $file->id(),
            'alt' => "",
            'title' => "",
          ];
        } else {
          $file_data = [];
        }
      } else {
        $file_data = [];
      }


      $node_export = Node::create([
        // The node entity bundle.
        'type' => 'other',
        'langcode' => 'en',
        'created' => REQUEST_TIME,
        'changed' => REQUEST_TIME,
        // The user ID.3

        'field_solutions' => $field_solutions,

        'field_country_region' => $field_country_region,

        'field_category_reference' => [],

        'field_date' => date('Y-m-d',$date),

        'field_image' => [
          $file_data,
        ],

        'uid' => 1,
        'title' => $values['data']->title,

        'body' => [
          'summary' => '',
          'value' => $body,
          'format' => 'full_html',
        ],

      ]);
      $node_export->save();
      \Drupal::service('path.alias_storage')->save("/node/" . $node_export->id(), "/" . $alias, "en");

    }
  }

  function export_d6_import_region_info() {
    // Switch to external database
    \Drupal\Core\Database\Database::setActiveConnection('ddd6');

    // Get a connection going
    $db = \Drupal\Core\Database\Database::getConnection();

    $sql = "SELECT
             n.nid,
             n.title,
             n.created,
             nr.body,
             nr.teaser,
             tn.tid,
             td.vid,
             u.dst

             FROM node n
             INNER JOIN node_revisions nr ON n.vid = nr.vid
             JOIN term_node tn ON n.nid = tn.nid
             JOIN term_data td ON tn.tid = td.tid
             INNER JOIN url_alias u ON u.src LIKE CONCAT('node/', n.nid)
             WHERE n.status = 1 AND n.type = 'region_info'
            ";

    $query = $db->query($sql)->fetchAll();

    \Drupal\Core\Database\Database::setActiveConnection('default');
    $db8 = \Drupal\Core\Database\Database::getConnection();

    foreach($query as $key => $val) {
      $info[$val->nid]['terms'][$val->vid][] = $val->tid;
      $info[$val->nid]['data'] = $val;
    }

    kint($info);


    foreach($info as $nid => $values) {

      $field_solutions = array();
      $field_country_region = array();

      if (!empty($values['terms'][1])) {
        foreach ($values['terms'][1] as $term_solution) {
          $field_solutions[]['target_id'] = $term_solution;
        }
      }

      if (!empty($values['terms'][3])) {
        foreach ($values['terms'][3] as $term_region) {
          $field_country_region[]['target_id'] = $term_region;
        }
      }

      $date = $values['data']->created;
      $title = $values['data']->title;
      $body = $values['data']->body;
      $teaser = $values['data']->teaser;
      $alias = $values['data']->dst;

      $node_export = Node::create([
        // The node entity bundle.
        'type' => 'other',
        'langcode' => 'en',
        'created' => REQUEST_TIME,
        'changed' => REQUEST_TIME,
        // The user ID.3

        'field_solutions' => $field_solutions,

        'field_country_region' => $field_country_region,

        'field_category_reference' => [],

        'field_date' => date('Y-m-d',$date),

        'uid' => 1,
        'title' => $title,

        'body' => [
          'summary' => $teaser,
          'value' => $body,
          'format' => 'full_html',
        ],

      ]);
      $node_export->save();
      \Drupal::service('path.alias_storage')->save("/node/" . $node_export->id(), "/" . $alias, "en");

    }

  }

}

