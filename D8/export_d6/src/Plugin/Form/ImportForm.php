<?php
/**
 * @file
 */

namespace Drupal\export_d6\Plugin\Form;

use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\export_d6\Plugin\Block;
use Drupal\export_d6;
use Drupal\export_d6\Controller\Export_d6Controller;

class ImportForm extends FormBase {

  public function getFormId() {
    return 'export_d6_form';
  }

  public function buildForm(array $form, FormStateInterface $form_state) {
    $checkbox_option = ['Regional Info', 'Story', 'Data', 'Page', 'News and Events', 'Blog', 'Taxonomy'];

    $form['content_type'] = array(
      '#type' => 'select',
      '#options' => array_combine($checkbox_option, $checkbox_option),
      '#title' => $this->t('Which content do you want to import?'),
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => $this->t('Import'),
    );
    return $form;
  }

  public function validateForm(array &$form, FormStateInterface $form_state) {
    // Validate submitted form data.
  }

  public function submitForm(array &$form, FormStateInterface $form_state) {
//    kint($form_state);
    $values = $form_state->getValues();
    $message = $values['content_type'] . ' successfully imported';
    switch($values['content_type']){
      case 'Blog' :
        $result = new Export_d6Controller();
        $result->export_d6_import();
        drupal_set_message($message);
        break;
      case 'Taxonomy' :
        $result = new Export_d6Controller();
        $result->export_d6_term_import();
        drupal_set_message($message);
        break;
      case 'News and Events' :
        $result = new Export_d6Controller();
        $result->export_d6_news_import();
        drupal_set_message($message);
        break;
      case 'Page' :
        $result = new Export_d6Controller();
        $result->export_d6_import_page();
        drupal_set_message($message);
        break;
      case 'Data' :
        $result = new Export_d6Controller();
        $result->export_d6_import_data();
        drupal_set_message($message);
        break;
      case 'Story' :
        $result = new Export_d6Controller();
        $result->export_d6_import_story();
        drupal_set_message($message);
        break;
      case 'Regional Info' :
        $result = new Export_d6Controller();
        $result->export_d6_import_region_info();
        drupal_set_message($message);
        break;
    }
  }

}