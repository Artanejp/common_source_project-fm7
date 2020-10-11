;;; qtdoc.el --- Qt documentation lookup from within Emacs

;; Copyright 2007 by Martin Blais

;; Authors: Martin Blais <blais@furius.ca>,
;; Revision: Revision: 4879 
;; Date: $Date: 2007-01-11 06:28:06 -0800 (Thu, 11 Jan 2007) $

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License version 2,
;; as published by the Free Software Foundation.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License version 2
;; along with this program and available at
;; http://docutils.sf.net/licenses/gpl.txt and at
;; http://www.gnu.org/licenses/gpl.txt.

;;; Commentary:

;; Lookup Qt documentation in a Browser from Emacs. Emacs tells your running web
;; browser to open up the appropriate page.

;;; Description

;;; Download

;;; BUGS

;;; History:

;;; Code:


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Bindings and hooks

(defgroup qtdoc nil "Qt Documentation Browser"
  :group 'wp
  :version "21.1"
  :link '(url-link "http://furius.ca/pubcode/conf/lib/elisp/qtdoc.el"))

(defvar qtdoc-html-root "/usr/share/doc/qt-4.2.3/html"
  "The directory where your documentation lies.
You will almost certainly have to customize this.")

(defvar qtdoc-docindex-string
   "
A  	QAbstractButton 		QDir 		QIntValidator 		QRegExpValidator 		QTabWidget
	QAbstractEventDispatcher 		QDirectPainter 		QIODevice 		QRegion 		QTcpServer
	QAbstractExtensionFactory 		QDirModel 		QItemDelegate 		QResizeEvent 		QTcpSocket
	QAbstractExtensionManager 		QDockWidget 		QItemEditorCreatorBase 		QResource 		QTemporaryFile
	QAbstractFileEngine 		QDomAttr 		QItemEditorFactory 		QRubberBand 		QTestEventList
	QAbstractFileEngineHandler 		QDomCDATASection 		QItemSelection 	S  	QScreen 		QTextBlock
	QAbstractFormBuilder 		QDomCharacterData 		QItemSelectionModel 		QScreenCursor 		QTextBlockFormat
	QAbstractGraphicsShapeItem 		QDomComment 		QItemSelectionRange 		QScreenDriverFactory 		QTextBlockGroup
	QAbstractItemDelegate 		QDomDocument 	K  	QKbdDriverFactory 		QScreenDriverPlugin 		QTextBlockUserData
	QAbstractItemModel 		QDomDocumentFragment 		QKbdDriverPlugin 		QScrollArea 		QTextBrowser
	QAbstractItemView 		QDomDocumentType 		QKeyEvent 		QScrollBar 		QTextCharFormat
	QAbstractListModel 		QDomElement 		QKeySequence 		QSemaphore 		QTextCodec
	QAbstractPrintDialog 		QDomEntity 	L  	QLabel 		QSessionManager 		QTextCodecPlugin
	QAbstractProxyModel 		QDomEntityReference 		QLatin1Char 		QSet 		QTextCursor
	QAbstractScrollArea 		QDomImplementation 		QLatin1String 		QSetIterator 		QTextDecoder
	QAbstractSlider 		QDomNamedNodeMap 		QLayout 		QSettings 		QTextDocument
	QAbstractSocket 		QDomNode 		QLayoutItem 		QSharedData 		QTextDocumentFragment
	QAbstractSpinBox 		QDomNodeList 		QLCDNumber 		QSharedDataPointer 		QTextEdit
	QAbstractTableModel 		QDomNotation 		QLibrary 		QShortcut 		QTextEncoder
	QAbstractTextDocumentLayout 		QDomProcessingInstruction 		QLibraryInfo 		QShortcutEvent 		QTextFormat
	QAccessible 		QDomText 		QLine 		QShowEvent 		QTextFragment
	QAccessibleBridge 		QDoubleSpinBox 		QLinearGradient 		QSignalMapper 		QTextFrame
	QAccessibleBridgePlugin 		QDoubleValidator 		QLineEdit 		QSignalSpy 		QTextFrameFormat
	QAccessibleEvent 		QDrag 		QLineF 		QSize 		QTextImageFormat
	QAccessibleInterface 		QDragEnterEvent 		QLinkedList 		QSizeF 		QTextInlineObject
	QAccessibleObject 		QDragLeaveEvent 		QLinkedListIterator 		QSizeGrip 		QTextLayout
	QAccessiblePlugin 		QDragMoveEvent 		QLinuxFbScreen 		QSizePolicy 		QTextLength
	QAccessibleWidget 		QDropEvent 		QList 		QSlider 		QTextLine
	QAction 		QDynamicPropertyChangeEvent 		QListIterator 		QSocketNotifier 		QTextList
	QActionEvent 	E  	QErrorMessage 		QListView 		QSortFilterProxyModel 		QTextListFormat
	QActionGroup 		QEvent 		QListWidget 		QSound 		QTextObject
	QApplication 		QEventLoop 		QListWidgetItem 		QSpacerItem 		QTextOption
	QAssistantClient 		QExtensionFactory 		QLocale 		QSpinBox 		QTextStream
	QAxAggregated 		QExtensionManager 	M  	QMacPasteboardMime 		QSplashScreen 		QTextTable
	QAxBase 	F  	QFile 		QMacStyle 		QSplitter 		QTextTableCell
	QAxBindable 		QFileDialog 		QMainWindow 		QSplitterHandle 		QTextTableFormat
	QAxFactory 		QFileIconProvider 		QMap 		QSqlDatabase 		QThread
	QAxObject 		QFileInfo 		QMapIterator 		QSqlDriver 		QThreadStorage
	QAxScript 		QFileOpenEvent 		QMatrix 		QSqlDriverCreator 		QTime
	QAxScriptEngine 		QFileSystemWatcher 		QMenu 		QSqlDriverCreatorBase 		QTimeEdit
	QAxScriptManager 		QFlag 		QMenuBar 		QSqlDriverPlugin 		QTimeLine
	QAxWidget 		QFlags 		QMessageBox 		QSqlError 		QTimer
B  	QBasicTimer 		QFocusEvent 		QMetaClassInfo 		QSqlField 		QTimerEvent
	QBitArray 		QFocusFrame 		QMetaEnum 		QSqlIndex 		QToolBar
	QBitmap 		QFont 		QMetaMethod 		QSqlQuery 		QToolBox
	QBoxLayout 		QFontComboBox 		QMetaObject 		QSqlQueryModel 		QToolButton
	QBrush 		QFontDatabase 		QMetaProperty 		QSqlRecord 		QToolTip
	QBuffer 		QFontDialog 		QMetaType 		QSqlRelation 		QTransformedScreen
	QButtonGroup 		QFontInfo 		QMimeData 		QSqlRelationalDelegate 		QTranslator
	QByteArray 		QFontMetrics 		QMimeSource 		QSqlRelationalTableModel 		QTreeView
	QByteArrayMatcher 		QFontMetricsF 		QModelIndex 		QSqlResult 		QTreeWidget
C  	QCache 		QFormBuilder 		QMotifStyle 		QSqlTableModel 		QTreeWidgetItem
	QCalendarWidget 		QFrame 		QMouseDriverFactory 		QStack 		QTreeWidgetItemIterator
	QCDEStyle 		QFSFileEngine 		QMouseDriverPlugin 		QStackedLayout 	U  	QUdpSocket
	QChar 		QFtp 		QMouseEvent 		QStackedWidget 		QUiLoader
	QCheckBox 	G  	QGenericArgument 		QMoveEvent 		QStandardItem 		QUndoCommand
	QChildEvent 		QGenericReturnArgument 		QMovie 		QStandardItemEditorCreator 		QUndoGroup
	QCleanlooksStyle 		QGLColormap 		QMultiHash 		QStandardItemModel 		QUndoStack
	QClipboard 		QGLContext 		QMultiMap 		QStatusBar 		QUndoView
	QCloseEvent 		QGLFormat 		QMutableHashIterator 		QStatusTipEvent 		QUrl
	QColor 		QGLFramebufferObject 		QMutableLinkedListIterator 		QString 		QUrlInfo
	QColorDialog 		QGLPixelBuffer 		QMutableListIterator 		QStringList 		QUuid
	QColormap 		QGLWidget 		QMutableMapIterator 		QStringListModel 	V  	QValidator
	QComboBox 		QGradient 		QMutableSetIterator 		QStringMatcher 		QVariant
	QCommonStyle 		QGraphicsEllipseItem 		QMutableVectorIterator 		QStyle 		QVarLengthArray
	QCompleter 		QGraphicsItem 		QMutex 		QStyleFactory 		QVBoxLayout
	QConicalGradient 		QGraphicsItemAnimation 		QMutexLocker 		QStyleHintReturn 		QVector
	QContextMenuEvent 		QGraphicsItemGroup 	N  	QNetworkAddressEntry 		QStyleHintReturnMask 		QVectorIterator
	QCopChannel 		QGraphicsLineItem 		QNetworkInterface 		QStyleOption 		QVFbScreen
	QCoreApplication 		QGraphicsPathItem 		QNetworkProxy 		QStyleOptionButton 		QVNCScreen
	QCursor 		QGraphicsPixmapItem 	O  	QObject 		QStyleOptionComboBox 	W  	QWaitCondition
	QCustomRasterPaintDevice 		QGraphicsPolygonItem 		QObjectCleanupHandler 		QStyleOptionComplex 		QWhatsThis
D  	QDataStream 		QGraphicsRectItem 	P  	QPageSetupDialog 		QStyleOptionDockWidget 		QWhatsThisClickedEvent
	QDataWidgetMapper 		QGraphicsScene 		QPaintDevice 		QStyleOptionFocusRect 		QWheelEvent
	QDate 		QGraphicsSceneContextMenuEvent 		QPaintEngine 		QStyleOptionFrame 		QWidget
	QDateEdit 		QGraphicsSceneEvent 		QPaintEngineState 		QStyleOptionFrameV2 		QWidgetAction
	QDateTime 		QGraphicsSceneHoverEvent 		QPainter 		QStyleOptionGraphicsItem 		QWidgetItem
	QDateTimeEdit 		QGraphicsSceneMouseEvent 		QPainterPath 		QStyleOptionGroupBox 		QWindowsMime
	QDBusAbstractAdaptor 		QGraphicsSceneWheelEvent 		QPainterPathStroker 		QStyleOptionHeader 		QWindowsStyle
	QDBusAbstractInterface 		QGraphicsSimpleTextItem 		QPaintEvent 		QStyleOptionMenuItem 		QWindowStateChangeEvent
	QDBusArgument 		QGraphicsSvgItem 		QPair 		QStyleOptionProgressBar 		QWindowsXPStyle
	QDBusConnection 		QGraphicsTextItem 		QPalette 		QStyleOptionProgressBarV2 		QWorkspace
	QDBusConnectionInterface 		QGraphicsView 		QPen 		QStyleOptionQ3DockWindow 		QWriteLocker
	QDBusError 		QGridLayout 		QPersistentModelIndex 		QStyleOptionQ3ListView 		QWSCalibratedMouseHandler
	QDBusInterface 		QGroupBox 		QPicture 		QStyleOptionQ3ListViewItem 		QWSClient
	QDBusMessage 	H  	QHash 		QPictureFormatPlugin 		QStyleOptionRubberBand 		QWSEmbedWidget
	QDBusObjectPath 		QHashIterator 		QPictureIO 		QStyleOptionSizeGrip 		QWSEvent
	QDBusReply 		QHBoxLayout 		QPixmap 		QStyleOptionSlider 		QWSInputMethod
	QDBusServer 		QHeaderView 		QPixmapCache 		QStyleOptionSpinBox 		QWSKeyboardHandler
	QDBusSignature 		QHelpEvent 		QPlastiqueStyle 		QStyleOptionTab 		QWSMouseHandler
	QDBusVariant 		QHideEvent 		QPluginLoader 		QStyleOptionTabBarBase 		QWSPointerCalibrationData
	QDecoration 		QHostAddress 		QPoint 		QStyleOptionTabV2 		QWSScreenSaver
	QDecorationFactory 		QHostInfo 		QPointer 		QStyleOptionTabWidgetFrame 		QWSServer
	QDecorationPlugin 		QHoverEvent 		QPointF 		QStyleOptionTitleBar 		QWSTslibMouseHandler
	QDesignerActionEditorInterface 		QHttp 		QPolygon 		QStyleOptionToolBar 		QWSWindow
	QDesignerContainerExtension 		QHttpHeader 		QPolygonF 		QStyleOptionToolBox 		QWSWindowSurface
	QDesignerCustomWidgetCollectionInterface 		QHttpRequestHeader 		QPrintDialog 		QStyleOptionToolButton 	X  	QX11EmbedContainer
	QDesignerCustomWidgetInterface 		QHttpResponseHeader 		QPrintEngine 		QStyleOptionViewItem 		QX11EmbedWidget
	QDesignerFormEditorInterface 	I  	QIcon 		QPrinter 		QStyleOptionViewItemV2 		QX11Info
	QDesignerFormWindowCursorInterface 		QIconDragEvent 		QProcess 		QStylePainter 		QXmlAttributes
	QDesignerFormWindowInterface 		QIconEngine 		QProgressBar 		QStylePlugin 		QXmlContentHandler
	QDesignerFormWindowManagerInterface 		QIconEnginePlugin 		QProgressDialog 		QSvgRenderer 		QXmlDeclHandler
	QDesignerMemberSheetExtension 		QImage 		QProxyModel 		QSvgWidget 		QXmlDefaultHandler
	QDesignerObjectInspectorInterface 		QImageIOHandler 		QPushButton 		QSyntaxHighlighter 		QXmlDTDHandler
	QDesignerPropertyEditorInterface 		QImageIOPlugin 	Q  	QQueue 		QSysInfo 		QXmlEntityResolver
	QDesignerPropertySheetExtension 		QImageReader 	R  	QRadialGradient 		QSystemLocale 		QXmlErrorHandler
	QDesignerTaskMenuExtension 		QImageWriter 		QRadioButton 		QSystemTrayIcon 		QXmlInputSource
	QDesignerWidgetBoxInterface 		QInputContext 		QRasterPaintEngine 	T  	QTabBar 		QXmlLexicalHandler
	QDesktopServices 		QInputContextFactory 		QReadLocker 		QTabletEvent 		QXmlLocator
	QDesktopWidget 		QInputContextPlugin 		QReadWriteLock 		QTableView 		QXmlNamespaceSupport
	QDial 		QInputDialog 		QRect 		QTableWidget 		QXmlParseException
	QDialog 		QInputEvent 		QRectF 		QTableWidgetItem 		QXmlReader
	QDialogButtonBox 		QInputMethodEvent 		QRegExp 		QTableWidgetSelectionRange 		QXmlSimpleReader
"
   "Cut-n-paste documentation index, to obtain the complete list of classes quick-n-dirty.")

(defvar qtdoc-classes
  (delete-if-not (lambda (x) (> (length x) 1)) 
		 (split-string qtdoc-docindex-string))
  "List of class names.")

(require 'iswitchb)

(defun qtdoc-completing-read (prompt choices)
  "Use iswitch as a completing-read replacement to choose from
choices. PROMPT is a string to prompt with. CHOICES is a list of
strings to choose from."
  (let ((iswitchb-make-buflist-hook
         (lambda ()
           (setq iswitchb-temp-buflist choices))))
    (iswitchb-read-buffer prompt)))


(require 'browse-url)

(defun qtdoc-lookup ()
  "Lookup a class name in the Qt documentation."
  (interactive)
  (let ((name (qtdoc-completing-read "Qt Class: " qtdoc-classes)))
    (if current-prefix-arg
	(browse-url (concat qtdoc-html-root "/" (downcase name) ".html") current-prefix-arg)
	(w3m (concat qtdoc-html-root "/" (downcase name) ".html") current-prefix-arg)
	)))

(provide 'qtdoc)
;;; qtdoc.el ends here
