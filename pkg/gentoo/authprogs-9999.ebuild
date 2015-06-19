# Copyright 1999-2011 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: Exp $

DESCRIPTION="ssh forced command authenticator"
HOMEPAGE="http://sourceforge.net/projects/authprogs/"
ESVN_REPO_URI="svn://svn.code.sf.net/p/authprogs/code/trunk"

inherit subversion

LICENSE="BSD"
SLOT="0"
KEYWORDS="x86 amd64"
IUSE=""

RDEPEND="virtual/ssh"
DEPEND="${RDEPEND}"
RESTRICT="mirror"
S="${WORKDIR}/authprogs-code"

src_compile() {
	emake || die
}

src_install() {
	dobin "${WORKDIR}/authprogs-code/src/authprogs"
}
