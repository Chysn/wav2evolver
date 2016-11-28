/*
 Navicat Premium Data Transfer

 Source Server         : Local
 Source Server Type    : MySQL
 Source Server Version : 50144
 Source Host           : localhost
 Source Database       : wav2evolver

 Target Server Type    : MySQL
 Target Server Version : 50144
 File Encoding         : utf-8

 Date: 01/24/2015 17:03:50 PM
*/

SET NAMES utf8;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
--  Table structure for `waveform`
-- ----------------------------
DROP TABLE IF EXISTS `waveform`;
CREATE TABLE `waveform` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `share_time` datetime DEFAULT NULL,
  `name` varchar(16) DEFAULT NULL,
  `description` varchar(140) DEFAULT NULL,
  `pcm` blob,
  `shared` tinyint(4) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=latin1;


SET FOREIGN_KEY_CHECKS = 1;
