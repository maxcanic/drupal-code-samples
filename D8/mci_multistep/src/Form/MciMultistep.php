<?php

namespace Drupal\mci_multistep\Form;

use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\taxonomy\Entity\Term;
use \Drupal\node\Entity\Node;
use \Drupal\file\Entity\File;


/**
 * Provides a form with two steps.
 *
 * This example demonstrates a multistep form with text input elements. We
 * extend FormBase which is the simplest form base class used in Drupal.
 *
 * @see \Drupal\Core\Form\FormBase
 */



class MciMultistep extends FormBase {

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'mci_multistep';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {

    if ($form_state->has('page_num') && $form_state->get('page_num') == 2) {
      return self::mci_multistep_Page_Two($form, $form_state);
    }
    elseif ($form_state->has('page_num') && $form_state->get('page_num') == 3) {
      return self::mci_multistep_Page_Three($form, $form_state);
    }

    $form_state->set('page_num', 1);

    $result_region = [];
    $result_country = [];
    $vid = 'regions';
    $tree = \Drupal::entityTypeManager()->getStorage('taxonomy_term')->loadTree($vid);
    foreach ($tree as $term) {
      //kint($term->parents[0]);
      if ($term->parents[0] == '0'){ //get only parent
        //$result_region[] = $term->name;
        $key = $term->tid;
        $value = $term->name;
        $result_region[$key] = $value;
        //kint($term);
      }
      elseif(!$term->parents[0] == '0'){//get children
        //$result_country[] = $term->name;
        $key = $term->tid;
        $value = $term->name;
        $result_country[$key] = $value;
        //kint($term);
      }
    }


    $form['description'] = [
      '#type' => 'item',
      '#title' => $this->t('A basic multistep form (page 1)'),
    ];

    $form['first_name'] = [
      '#type' => 'textfield',
      '#title' => $this->t('First Name'),
      '#description' => $this->t('Enter your first name.'),
      '#default_value' => $form_state->getValue('first_name', ''),
      '#required' => TRUE,
    ];

    $form['last_name'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Last Name'),
      '#default_value' => $form_state->getValue('last_name', ''),
      '#description' => $this->t('Enter your last name.'),
    ];

    $form['regions'] = array(
      '#title' => t('Regions'),
      '#type' => 'select',
      '#description' => 'Select your region',
      '#default_value' => 15,
      '#options' => $result_region,
      '#ajax' => array(
        'callback' => '::returnAjax',
        'wrapper' => 'edit-fieldsset',
        'method' => 'replace',
        'effect' => 'fade',
        'event' => 'change',
      ),
      '#suffix' => '</div>'
    );



    $form['country'] = array(
      '#title' => t('Countries'),
      '#type' => 'select',
      '#description' => 'Select your country',
      '#prefix' => '<div id="edit-fieldsset">',
      '#suffix' => '</div>',
      '#states' => array(
        'visible' => array(
          // Element to check => Condition to check
          ':input[name="regions"]' => array('value' => ''),
        ),
      ),
    );


    // Group submit handlers in an actions element with a key of "actions" so
    // that it gets styled correctly, and so that other modules may add actions
    // to the form. This is not required, but is convention.
    $form['actions'] = [
      '#type' => 'actions',
    ];




    $form['actions']['next'] = [
      '#type' => 'submit',
      '#button_type' => 'primary',
      '#value' => $this->t('Next'),
      // Custom submission handler for page 1.
      '#submit' => ['::mci_multistep_Form_Page_1_Submit'],
      // Custom validation handler for page 1.
      //'#validate' => ['::mci_multistep_get_countries($form_state)'],
    ];

    return $form;
  }

  function returnAjax($form, FormStateInterface $form_state) {
    $result_countries = [];
    $y = $form_state->getValue('regions');
    //$y = $page_values_1['region'];
    $vid = 'regions';
    $children = \Drupal::entityManager()->getStorage('taxonomy_term')->loadChildren($y,$vid);

    foreach ($children as $term) {
      $key = $term->id();
      $value = $term->getName();
      $result_countries[$key] = $value;
    }
    $form['country']['#options'] = $result_countries;

    if(!empty($result_countries)){
      $form['country']['#states'] = [
        'visible' => [
          ':input[name="regions"]' => array('value' => ''),
        ]
      ];
      return $form['country'];
    }
    else{
      $form['country']['#states'] = [
        'invisible' => [
          ':input[name="regions"]' => array('value' => ''),
        ]
      ];
      return;
    }
  }
  /**
   * Provides custom submission handler for page 1.
   *
   * @param array $form
   *   An associative array containing the structure of the form.
   * @param \Drupal\Core\Form\FormStateInterface $form_state
   *   The current state of the form.
   */
  public function mci_multistep_Form_Page_1_Submit(array &$form, FormStateInterface $form_state) {
    $form_state
      ->set('page_values_1', [
        // Keep only first step values to minimize stored data.
        'first_name' => $form_state->getValue('first_name'),
        'last_name' => $form_state->getValue('last_name'),
        'region' => $form_state->getValue('regions'),
        'country' => $form_state->getValue('country'),

      ])
      ->set('page_num', 2)
      ->setRebuild(TRUE);

  }



  /**
   * Builds the second step form (page 2).
   *
   * @param array $form
   *   An associative array containing the structure of the form.
   * @param \Drupal\Core\Form\FormStateInterface $form_state
   *   The current state of the form.
   *
   * @return array
   *   The render array defining the elements of the form.
   */
  public function mci_multistep_Page_Two(array &$form, FormStateInterface $form_state) {

    if ($form_state->has('page_num') && $form_state->get('page_num') == 3) {
      return self::mci_multistep_Page_Three($form, $form_state);
    }


    $form['description'] = [
      '#type' => 'item',
      '#title' => $this->t('A basic multistep form (page 2)'),
    ];

    $form['color'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Favorite color'),
      '#required' => TRUE,
      '#default_value' => $form_state->getValue('color', ''),
    ];

    kint($form_state);

    // Group submit handlers in an actions element with a key of "actions" so
    // that it gets styled correctly, and so that other modules may add actions
    // to the form. This is not required, but is convention.
    $form['actions'] = [
      '#type' => 'actions',
    ];

    $form['back'] = [
      '#type' => 'submit',
      '#value' => $this->t('Back'),
      // Custom submission handler for 'Back' button.
      '#submit' => ['::mci_multistep_Page_Two_Back'],
      // We won't bother validating the required 'color' field, since they
      // have to come back to this page to submit anyway.
      '#limit_validation_errors' => [],
    ];

    $form['actions']['next'] = [
      '#type' => 'submit',
      '#button_type' => 'primary',
      '#value' => $this->t('Next'),
      // Custom submission handler for page 1.
      '#submit' => ['::mci_multistep_Form_Page_2_Submit'],
      // Custom validation handler for page 1.
      '#validate' => ['::mci_multistep_Form_Page_2_Validate'],
    ];

    return $form;
  }


  /**
   * Provides custom submission handler for 'Back' button (page 2).
   *
   * @param array $form
   *   An associative array containing the structure of the form.
   * @param \Drupal\Core\Form\FormStateInterface $form_state
   *   The current state of the form.
   */
  public function mci_multistep_Page_Two_Back(array &$form, FormStateInterface $form_state) {
    $form_state
      // Restore values for the first step.
      ->setValues($form_state->get('page_values_1'))
      ->set('page_num', 1)
      ->setRebuild(TRUE);
  }



  public function mci_multistep_Form_Page_2_Validate(array &$form, FormStateInterface $form_state) {
    $color = $form_state->getValue('color');
    if ($color = '' ) {
      // Set an error for the form element with a key of "birth_year".
      $form_state->setErrorByName('color', $this->t('Enter a color'));
    }
  }

  /**
   * Provides custom submission handler for page 2.
   *
   * @param array $form
   *   An associative array containing the structure of the form.
   * @param \Drupal\Core\Form\FormStateInterface $form_state
   *   The current state of the form.
   */
  public function mci_multistep_Form_Page_2_Submit(array &$form, FormStateInterface $form_state) {

    $form_state
      ->set('page_values_2', [
        // Keep only first step values to minimize stored data.
        'color' => $form_state->getValue('color')
      ])
      ->set('page_num', 3)
      ->setRebuild(TRUE);
  }


  /**
   * Builds the third step form (page 3).
   *
   * @param array $form
   *   An associative array containing the structure of the form.
   * @param \Drupal\Core\Form\FormStateInterface $form_state
   *   The current state of the form.
   *
   * @return array
   *   The render array defining the elements of the form.
   */
  public function mci_multistep_Page_Three(array &$form, FormStateInterface $form_state) {

    $form['description'] = [
      '#type' => 'item',
      '#title' => $this->t('A basic multistep form (page 3)'),
    ];

    $form['birth_year'] = [
      '#type' => 'number',
      '#title' => $this->t('Birth Year'),
      '#default_value' => $form_state->getValue('birth_year', ''),
      '#description' => $this->t('Format is "YYYY" and value between 1900 and 2000'),
    ];

    $form['back'] = [
      '#type' => 'submit',
      '#value' => $this->t('Back'),
      // Custom submission handler for 'Back' button.
      '#submit' => ['::mci_multistep_Page_Three_Back'],
      // We won't bother validating the required 'color' field, since they
      // have to come back to this page to submit anyway.
      '#limit_validation_errors' => [],
    ];
    $form['submit'] = [
      '#type' => 'submit',
      '#button_type' => 'primary',
      '#value' => $this->t('Submit'),
      '#validate' => ['::mci_multistep_Form_Page_3_Validate'],
    ];

    return $form;
  }

  /**
   * Provides custom submission handler for 'Back' button (page 3).
   *
   * @param array $form
   *   An associative array containing the structure of the form.
   * @param \Drupal\Core\Form\FormStateInterface $form_state
   *   The current state of the form.
   */
  public function mci_multistep_Page_Three_Back(array &$form, FormStateInterface $form_state) {
    $form_state
      // Restore values for the first step.
      ->setValues($form_state->get('page_values_2'))
      ->set('page_num', 2)
      ->setRebuild(TRUE);
  }

  /**
   * Provides custom validation handler for page 3.
   *
   * @param array $form
   *   An associative array containing the structure of the form.
   * @param \Drupal\Core\Form\FormStateInterface $form_state
   *   The current state of the form.
   */
  public function mci_multistep_Form_Page_3_Validate(array &$form, FormStateInterface $form_state) {
    $birth_year = $form_state->getValue('birth_year');

    if ($birth_year != '' && ($birth_year < 1900 || $birth_year > 2000)) {
      // Set an error for the form element with a key of "birth_year".
      $form_state->setErrorByName('birth_year', $this->t('Enter a year between 1900 and 2000.'));
    }
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {

    $page_values_1 = $form_state->get('page_values_1');
    $page_values_2 = $form_state->get('page_values_2');

    $region = \Drupal::entityTypeManager()->getStorage('taxonomy_term')->load($page_values_1['region']);
    $country = \Drupal::entityTypeManager()->getStorage('taxonomy_term')->load($page_values_1['country']);



    $region_id = Term::load($page_values_1['region']);
    $region = $region_id->getName();
    $country_id = Term::load($page_values_1['country']);
    if(!empty($country_id)){
      $country = $country_id->getName();
    }
    else{
      $country = 'No country in this region.';
    }



    //$region = $region->name;
    //$country = $country->name;
    $title = $page_values_1['first_name'] . ' ' . $page_values_1['last_name'];

    $node = Node::create([
      'type'        => 'user_info',
      'title'       => $title,
      'field_favorite_color' => $page_values_2['color'],
      'field_last_name' => $page_values_1['last_name'],
      'field_name' => $page_values_1['first_name'],
      'field_region' => [$region_id],
      'field_country' => [$country_id]
    ]);
    $node->save();

    drupal_set_message($this->t('The form has been submitted. name="@first @last"', [
      '@first' => $page_values_1['first_name'],
      '@last' => $page_values_1['last_name']
    ]));

    drupal_set_message($this->t('Favorite color is: "@color", your region is: "@region" , and the country in that region is: "@country" ', [
      '@color' => $page_values_2['color'],
      '@region' => $region,
      '@country' => $country
    ]));

    drupal_set_message($this->t('And the year of birth is @birth_year', ['@birth_year' => $form_state->getValue('birth_year')]));
  }



}
