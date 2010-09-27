#include "termwidgetholder.h"
#include "termwidget.h"


TermWidgetHolder::TermWidgetHolder(const QString & wdir, QWidget * parent)
    : QWidget(parent),
      m_wdir(wdir)
{
    QGridLayout * lay = new QGridLayout(this);
    lay->setSpacing(0);
    lay->setContentsMargins(0, 0, 0, 0);

    QSplitter * s = new QSplitter(this);
    s->addWidget(newTerm());
    lay->addWidget(s);

    setLayout(lay);
}

TermWidgetHolder::~TermWidgetHolder()
{
}

TermWidget * TermWidgetHolder::terminal()
{
    QList<TermWidget*> list = findChildren<TermWidget*>();
    return list.count() == 0 ? 0 : list.at(0);
}

void TermWidgetHolder::setWDir(const QString & wdir)
{
    m_wdir = wdir;
}

void TermWidgetHolder::propertiesChanged()
{
    foreach(TermWidget *w, findChildren<TermWidget*>())
        w->propertiesChanged();
}

void TermWidgetHolder::splitHorizontal(TermWidget * term)
{
    split(term, Qt::Horizontal);
}

void TermWidgetHolder::splitVertical(TermWidget * term)
{
    split(term, Qt::Vertical);
}

void TermWidgetHolder::splitCollapse(TermWidget * term)
{
    QSplitter * parent = qobject_cast<QSplitter*>(term->parent());
    assert(parent);
    term->setParent(0);
    delete term;

    int cnt = parent->findChildren<TermWidget*>().count();
    if (cnt == 0)
    {
        parent->setParent(0);
        delete parent;
    }
    
    emit enableCollapse( findChildren<TermWidget*>().count() > 1 );
    update();
}

void TermWidgetHolder::split(TermWidget * term, Qt::Orientation orientation)
{
    QSplitter * parent = qobject_cast<QSplitter*>(term->parent());
    assert(parent);

    int ix = parent->indexOf(term);
    QList<int> parentSizes = parent->sizes();

    QList<int> sizes;
    sizes << 1 << 1;

    QSplitter * s = new QSplitter(orientation, this);
    s->insertWidget(0, term);
    s->insertWidget(1, newTerm());
    s->setSizes(sizes);

    parent->insertWidget(ix, s);
    parent->setSizes(parentSizes);
}

TermWidget * TermWidgetHolder::newTerm()
{
     TermWidget * w = new TermWidget(m_wdir, this);
     // proxy signals
     connect(w, SIGNAL(renameSession()), this, SIGNAL(renameSession()));
     connect(w, SIGNAL(removeCurrentSession()), this, SIGNAL(lastTerminalClosed()));
     // consume signals
     connect(w, SIGNAL(splitHorizontal(TermWidget *)),
             this, SLOT(splitHorizontal(TermWidget *)));
     connect(w, SIGNAL(splitVertical(TermWidget *)),
             this, SLOT(splitVertical(TermWidget *)));
     connect(w, SIGNAL(splitCollapse(TermWidget *)),
             this, SLOT(splitCollapse(TermWidget *)));
     // backward signals
     connect(this, SIGNAL(enableCollapse(bool)), w, SLOT(enableCollapse(bool)));

     emit enableCollapse( findChildren<TermWidget*>().count() > 1 ); 

     return w;
}



