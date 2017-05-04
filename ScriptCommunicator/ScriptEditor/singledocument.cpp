#include "singledocument.h"
#include "Qsci/qscilexerjavascript.h"
#include "mainwindow.h"
#include <Qsci/qsciapis.h>
#include <QTextStream>
#include <QCoreApplication>
#include <QDirIterator>
#include <QThread>
#include "parseThread.h"




/**
 * Constructor.
 * @param mainWindow
 *      Pointer to the main window.
 * @param parent
 *      The parent.
 */
SingleDocument::SingleDocument(MainWindow *mainWindow, QWidget *parent) :
    QsciScintilla(parent), m_mainWindow(mainWindow), m_documentName(""), m_fileLastModified(QDateTime::currentDateTime()),
    m_fileMustBeParsed(true), m_clickIndicatorStart(-1), m_clickIndicatorEnd(-1), m_clickIndicatorIdentifier(1)
{

    connect(this, SIGNAL(textChanged()), m_mainWindow, SLOT(documentWasModified()));

    m_clickIndicatorIdentifier = indicatorDefine(PlainIndicator, -1);
    setIndicatorForegroundColor(QColor(0,0,255), m_clickIndicatorIdentifier);

    setUtf8(true);
}

/**
 * Underlines a word which can be clicked (funktion or variable in the outline window).
 * @param pos
 *      The position of one character of the word.
 */
void SingleDocument::underlineWordWhichCanBeClicked(int pos)
{
    if(m_clickIndicatorStart != -1)
    {
        removeUndlineFromWordWhichCanBeClicked();
    }

    m_clickIndicatorStart = SendScintilla(SCI_WORDSTARTPOSITION, pos, true);
    m_clickIndicatorEnd = SendScintilla(SCI_WORDENDPOSITION, pos, true);

    fillIndicatorRangeWithPosition(m_clickIndicatorStart, m_clickIndicatorEnd, m_clickIndicatorIdentifier);
}

/**
 * Clears the current underline (clickable word).
 */
void SingleDocument::removeUndlineFromWordWhichCanBeClicked(void)
{
    if(m_clickIndicatorStart != -1)
    {
        clearIndicatorRangeWithPosition(m_clickIndicatorStart, m_clickIndicatorEnd, m_clickIndicatorIdentifier);
        m_clickIndicatorStart = -1;
        m_clickIndicatorEnd = -1;
    }
}

/**
 * Returns the last modified time stamp.
 */
QDateTime SingleDocument::getLastModified(void)
{
    return m_fileLastModified;
}

/**
 * Updates the last modified time stamp.
 */
void SingleDocument::updateLastModified(void)
{
    QFileInfo fileInfo(m_documentName);
    m_fileLastModified = fileInfo.lastModified();
}

/**
 * Sets the document name/path.
 * @param name
 *      The document name.
 */
void SingleDocument::setDocumentName(QString name, QFont font)
{
    m_documentName = name;
    initLexer(name);
    lexer()->setFont(font, -1);
}

/**
 * Initializes the lexer.
 * @param script
 *      The script file.
 */
void SingleDocument::initLexer(QString script)
{
    (void)script;

    if(lexer() == 0)
    {
        setLexer(new QsciLexerJavaScript(this));
        QsciAPIs* apis = new QsciAPIs(lexer());
        (void)apis;


        setAutoCompletionSource(QsciScintilla::AcsAPIs);
        setAutoCompletionCaseSensitivity(false);
        setAutoCompletionThreshold(3);
        setAutoIndent(true);
        setIndentationWidth(4);
        setTabWidth(4);
        setMarginLineNumbers(0,true);
        setMarginType(0, QsciScintilla::NumberMargin);
        setFolding(QsciScintilla::CircledTreeFoldStyle);
        setMarginsForegroundColor(QColor(128, 128, 128));

        dynamic_cast<QsciLexerJavaScript*>(lexer())->setFoldComments(true);
        dynamic_cast<QsciLexerJavaScript*>(lexer())->setFoldCompact(false);
       }


}


/**
 * Initializes the autocompletion.
 * @param additionalElements
 *      Additional elements for the autocompletion api.
 */
void SingleDocument::initAutoCompletion(QStringList& additionalElements, QMap<QString, QStringList>& autoCompletionEntries,
                                        QMap<QString, QStringList>& autoCompletionApiFiles)
{


    //Initialize the api.
    if(lexer() && lexer()->apis())
    {
        QsciAPIs* apis = static_cast<QsciAPIs*>(lexer()->apis());
        apis->clear();
        QMap<QString, QStringList>::iterator i;
        for (i = autoCompletionEntries.begin(); i != autoCompletionEntries.end(); ++i)
        {
            for(auto el : i.value())
            {
                apis->add(el);
            }
        }

        for (i = autoCompletionApiFiles.begin(); i != autoCompletionApiFiles.end(); ++i)
        {
            for(auto el : i.value())
            {
                apis->add(el);
            }
        }

        for(auto el : additionalElements)
        {
            apis->add(el);
        }
        apis->prepare();

    }

}
