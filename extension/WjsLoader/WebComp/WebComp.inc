<?php

namespace Wjs\Loader;

use \Wjs\Loader;

/**
 * @package Wjs\Loader
 */
class WebComp extends Loader {
  var $allowRequireJs = TRUE;
  var $requireCssLinks = TRUE;
  var $requireJsLinks = TRUE;

  function extensionAdd($name, $data) {
    // If data is only string it's should be the path of
    // server folder, who contains a .conf ini file.
    if (!is_array($data)) {
      $data = array(
        'server' => $data,
      );
    }
    if (!isset($data['client'])) {
      // Define path according wjs, this path can be wrong
      // if component is accessed into a rewritten client path.
      $data['client'] = $this->wjs->pathClient(realpath($data['server']) . '/');
    }
    // Do not add requirements as part of data.
    $require = FALSE;
    if (isset($data['require'])) {
      $require = $data['require'];
      unset($data['require']);
    }
    // Load sub extensions
    $extension = $data['server'] . '/extension/';
    if (is_dir($extension)) {
      $this->wjs->scanFolderExtensions($extension);
    }
    // Normal add.
    parent::extensionAdd($name, $data);
    // Load requirements.
    if ($require) {
      $this->wjs->extensionAddRequireArray($this->type, $name, $require);
    }
  }

  /**
   * Get content of WebComp. HTML file is loaded,
   * image links are parsed, CSS and JS requirements
   * are added as normal extensions.
   */
  function extensionPush($name, $options = array(), $value = NULL) {
    $value = $this->valuePush($name, $value);
    if (!$this->valueIsError($value)) {
      if (is_dir($value['server'])) {
        $value         = array_merge($value, array(
          // By default destination is not defined.
          'dom'   => isset($value['dom']) ? $value['dom'] : FALSE,
          // Html can come from outside.
          'html'  => isset($value['html']) ? $value['html'] : FALSE,
          // We can choose to display only one WebComp of each group at a time.
          'group' => isset($value['group']) ? $value['group'] : FALSE,
        ));
        $value['html'] = $this->htmlLoad($name, $options, $value);
        // Html can be missing.
        if ($value['html'] !== '') {
          // Rebase links / images to web component folder.
          $value['html'] = $this->wjs->htmlRebase($value['html'], $value['client']);
          $this->wjs->htmlParseIncludeRequire($value['html'], $this->type, $name);
        }
        // Search for extra css file,
        // but first search for a min.css version.
        if ($this->requireCssLinks && !$this->wjs->requireLinksClassFile(realpath($value['server']) . '/' . $name . '.min.css', 'css', $this->type, $name)) {
          // Fallback to .css version if no min.
          $this->wjs->requireLinksClassFile(realpath($value['server']) . '/' . $name . '.css', 'css', $this->type, $name);
        }
        if ($this->requireJsLinks) {
          // Search for extra js file.
          $this->wjs->requireLinksClassFile(realpath($value['server']) . '/' . $name . '.js', 'js', $this->type, $name);
        }
        // Remove unsafe data.
        unset($value['server']);
      }
    }
    // Save value as true.
    return parent::extensionPush($name, $options, $value);
  }

  function htmlLoad($name, $options, $value) {
    $htmlFile = $value['server'] . '/' . $name . '.html';
    if (is_file($htmlFile)) {
      return file_get_contents($htmlFile);
    }
    return '';
  }

  /**
   * Register WebComp from a folder content.
   */
  function scanFolderItem($directory, $item, $globalData) {
    if (is_dir($directory . $item)) {
      // Append global data.
      $this->wjs->extensionAdd($this->type, $item, $directory . $item . '/', $globalData);
    }
  }

  function query($group) {
    if (isset($_GET, $_GET[$this->type], $_GET[$this->type][$group])) {
      $this->wjs->import($this->type, $_GET[$this->type][$group]);
      return TRUE;
    }
    return FALSE;
  }
}